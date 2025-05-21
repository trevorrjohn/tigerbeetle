const std = @import("std");
const vsr = @import("vsr");
const exports = vsr.tb_client.exports;
const type_mappings = vsr.tb_client.type_mappings;
const assert = std.debug.assert;

// const ruby = @cImport(@cInclude("ruby/ruby.h"));
// const tb_client = @cImport(@cInclude("./ext/tb_client.h"));
//
const constants = vsr.constants;
const IO = vsr.io.IO;

const Tracer = vsr.trace.TracerType(vsr.time.Time);
const Storage = vsr.storage.StorageType(IO, Tracer);
const StateMachine = vsr.state_machine.StateMachineType(Storage, constants.state_machine_config);
const tb = vsr.tigerbeetle;

/// VSR type mappings: these will always be the same regardless of state machine.
const mappings_vsr = .{
    .{ exports.tb_operation, "Operation" },
    .{ exports.tb_packet_status, "PacketStatus" },
    .{ exports.tb_packet_t, "Packet", "tb_packet" },
    .{ exports.tb_client_t, "Client", "tb_client" },
    .{ exports.tb_init_status, "InitStatus" },
    .{ exports.tb_client_status, "ClientStatus" },
    .{ exports.tb_log_level, "LogLevel" },
    .{ exports.tb_register_log_callback_status, "RegisterLogCallbackStatus" },
};

/// State machine specific mappings: in future, these should be pulled automatically from the state
/// machine.
const mappings_state_machine = .{
    .{ tb.AccountFlags, "AccountFlags" },
    .{ tb.TransferFlags, "TransferFlags" },
    .{ tb.AccountFilterFlags, "AccountFilterFlags" },
    .{ tb.QueryFilterFlags, "QueryFilterFlags" },
    .{ tb.Account, "Account", "tb_account" },
    .{ tb.Transfer, "Transfer", "tb_transfer" },
    .{ tb.CreateAccountResult, "CreateAccountResult", "tb_create_accounts_result" },
    .{ tb.CreateTransferResult, "CreateTransferResult", "tb_create_transfer_result" },
    .{ tb.AccountFilter, "AccountFilter", "tb_account_filter" },
    .{ tb.AccountBalance, "AccountBalance", "tb_account_balance" },
    .{ tb.QueryFilter, "QueryFilter", "tb_query_filter" },
};

const mappings_all = mappings_vsr ++ mappings_state_machine;

const Buffer = struct {
    inner: std.ArrayList(u8),

    pub fn init(allocator: std.mem.Allocator) Buffer {
        return .{
            .inner = std.ArrayList(u8).init(allocator),
        };
    }

    pub fn print(self: *Buffer, comptime format: []const u8, args: anytype) void {
        self.inner.writer().print(format, args) catch unreachable;
    }
};

fn mapping_name_from_type(mappings: anytype, Type: type) ?[]const u8 {
    comptime for (mappings) |mapping| {
        const ZigType = mapping[0];
        if (Type == ZigType) {
            return mapping[1];
        }
    };
    return null;
}

fn zig_to_ruby(comptime Type: type) []const u8 {
    switch (@typeInfo(Type)) {
        .Array => |info| {
            return std.fmt.comptimePrint("[{s}, {d}]", .{
                comptime zig_to_ruby(info.child),
                info.len,
            });
        },
        .Enum => return comptime mapping_name_from_type(mappings_state_machine, Type).?,
        .Struct => return comptime mapping_name_from_type(mappings_state_machine, Type).?,
        .Bool => return ":bool",
        .Int => |info| {
            assert(info.signedness == .unsigned);

            return switch (info.bits) {
                8 => ":uint8",
                16 => ":uint16",
                32 => ":uint32",
                64 => ":uint64",
                128 => ":uint128",
                else => @compileError("invalid int type"),
            };
        },
        .Optional => |info| switch (@typeInfo(info.child)) {
            .Pointer => return zig_to_ruby(@typeInfo(info.child)),
            else => @compileError("Unsupported optional type: " ++ @typeName(Type)),
        },
        .Pointer => |info| {
            assert(info.size == .One);
            assert(!info.is_allowzero);

            return ":pointer";
        },
        .Void => return ":void",
        else => @compileError("Unhandled type: " ++ @typeName(Type)),
    }
}

fn to_uppercase(comptime input: []const u8) [input.len]u8 {
    comptime var output: [input.len]u8 = undefined;
    inline for (&output, 0..) |*char, i| {
        char.* = input[i];
        char.* -= 32 * @as(u8, @intFromBool(char.* >= 'a' and char.* <= 'z'));
    }
    return output;
}

fn ruby_ffi_enum_type(comptime Type: type) []const u8 {
    return switch (@bitSizeOf(Type)) {
        8 => "FFI::Type::UINT8",
        16 => "FFI::Type::UINT16",
        32 => "FFI::Type::UINT32",
        64 => "FFI::Type::UINT64",
        else => @compileError("invalid int type"),
    };
}

fn ruby_macro_accessor(comptime Type: type) []const u8 {
    switch (@typeInfo(type)) {
        .Bool => return "INIT_BOOL_ACCESSORS",
        .Int => switch (@bitSizeOf(Type)) {
            8 => return "INIT_UINT_ACCESSORS",
            16 => return "INIT_UINT_ACCESSORS",
            32 => return "INIT_UINT_ACCESSORS",
            64 => return "INIT_UINT_ACCESSORS",
            128 => return "INIT_UINT128_ACCESSORS",
            else => @compileError("Unsupported int bit size: " ++ std.fmt.comptimePrint("{d}", .{@bitSizeOf(Type)})),
        },
        else => @compileError("Unsupported type: " ++ @typeName(Type)),
    }
}

fn emit_enum(
    buffer: *Buffer,
    comptime Type: type,
    comptime type_info: anytype,
    comptime ruby_name: []const u8,
    comptime skip_fields: []const []const u8,
) !void {
    buffer.print("  VALUE m{s} = rb_define_module_under(module, \"{s}\");\n", .{ ruby_name, ruby_name });

    inline for (type_info.fields, 0..) |field, i| {
        if (comptime std.mem.startsWith(u8, field.name, "deprecated_")) continue;
        comptime var skip = false;
        inline for (skip_fields) |sf| {
            skip = skip or comptime std.mem.eql(u8, sf, field.name);
        }

        if (!skip) {
            const field_name = to_uppercase(field.name);
            if (@typeInfo(Type) == .Enum) {
                buffer.print("  rb_define_const(m{s}, \"{s}\", INT2NUM({d}));\n", .{
                    ruby_name,
                    @as([]const u8, &field_name),
                    @intFromEnum(@field(Type, field.name)),
                });
            } else {
                buffer.print("  rb_define_const(m{s}, \"{s}\", INT2NUM(1 << {d}));\n", .{
                    ruby_name,
                    @as([]const u8, &field_name),
                    i,
                });
            }
        }
    }
    buffer.print("\n", .{});
}

fn zig_type_to_ctype(comptime Type: type) []const u8 {
    switch (@typeInfo(Type)) {
        .Array => |info| {
            return std.fmt.comptimePrint("[{s}, {d}]", .{
                comptime zig_to_ruby(info.child),
                info.len,
            });
        },
        .Enum => return comptime mapping_name_from_type(mappings_state_machine, Type).?,
        .Struct => return comptime mapping_name_from_type(mappings_state_machine, Type).?,
        .Int => |info| {
            assert(info.signedness == .unsigned);

            return switch (info.bits) {
                8 => "uint8_t",
                16 => "uint16_t",
                32 => "uint32_t",
                64 => "uint64_t",
                128 => "tb_uint128_t",
                else => @compileError("invalid int type"),
            };
        },
        .Pointer => |info| {
            assert(info.size == .One);
            assert(!info.is_allowzero);

            return "void*";
        },
        .Void => return "void",
        else => @compileError("Unhandled type: " ++ @typeName(Type)),
    }
}

fn emit_struct_native_extensions(
    buffer: *Buffer,
    comptime type_info: std.builtin.Type.Struct,
    comptime ruby_name: []const u8,
    comptime tb_struct: []const u8,
) !void {
    buffer.print("// {s} - {s}\n", .{ ruby_name, tb_struct });
    buffer.print("DEFINE_RB_CLASS_FOR_STRUCT({s})\n", .{tb_struct});
    inline for (type_info.fields) |field| {
        if (comptime std.mem.startsWith(u8, field.name, "deprecated_")) continue;
        if (comptime std.mem.eql(u8, field.name, "reserved")) continue;
        if (comptime std.mem.eql(u8, field.name, "opaque")) continue;

        switch (@typeInfo(field.type)) {
            .Array => |info| {
                buffer.print("DEFINE_BYTE_ARRAY_ACCESSORS({s}, {s}, {s}, {d})", .{
                    tb_struct,
                    field.name,
                    comptime zig_type_to_ctype(info.child),
                    info.len,
                });
            },
            .Enum => |info| {
                const tag_type = info.tag_type;
                switch (@typeInfo(tag_type)) {
                    .Int => {
                        const ctype = comptime zig_type_to_ctype(tag_type);
                        buffer.print("DEFINE_UINT_ACCESSORS({s}, {s}, {s})\n", .{
                            tb_struct,
                            ctype,
                            field.name,
                        });
                    },
                    else => @compileError("Unsupported enum type: " ++ @typeName(tag_type)),
                }
            },
            .Int => |info| {
                assert(info.signedness == .unsigned);

                const ctype = comptime zig_type_to_ctype(field.type);
                if (comptime std.mem.eql(u8, "tb_uint128_t", ctype)) {
                    buffer.print("DEFINE_UINT128_ACCESSORS({s}, {s})\n", .{ tb_struct, field.name });
                } else {
                    buffer.print("DEFINE_UINT_ACCESSORS({s}, {s}, {s})\n", .{
                        tb_struct,
                        ctype,
                        field.name,
                    });
                }
            },
            .Optional => |info| switch (@typeInfo(info.child)) {
                .Pointer => {
                    buffer.print("DEFINE_VOID_PTR_ACCESSORS({s}, {s})\n", .{
                        tb_struct,
                        field.name,
                    });
                },
                else => @compileError("Unsupported optional type: " ++ @typeName(field.type)),
            },
            .Struct => |info| switch (info.layout) {
                .auto => @compileError("Invalid struct type: " ++ @typeName(field.type)),
                .@"packed" => {
                    const backing_int = info.backing_integer.?;
                    const ctype = comptime zig_type_to_ctype(backing_int);
                    buffer.print("DEFINE_UINT_ACCESSORS({s}, {s}, {s})\n", .{
                        tb_struct,
                        ctype,
                        field.name,
                    });
                },
                .@"extern" => continue,
            },
            else => @compileError("Unhandled type: " ++ field.name),
        }
    }
    buffer.print("\n\n", .{});
}

fn emit_ruby_method_accessors(
    buffer: *Buffer,
    comptime type_info: std.builtin.Type.Struct,
    comptime ruby_name: []const u8,
    comptime tb_struct: []const u8,
) !void {
    const ruby_class = "m_" ++ ruby_name ++ "_klass";
    buffer.print(
        \\
        \\  VALUE {s} = rb_define_class_under(module, "{s}", rb_cObject);
        \\  rb_define_alloc_func({s}, rb_{s}_alloc);
        \\  rb_define_method({s}, "initialize", rb_{s}_initialize, -1);
        \\
    , .{ ruby_class, ruby_name, ruby_class, tb_struct, ruby_class, tb_struct });
    inline for (type_info.fields) |field| {
        if (comptime std.mem.startsWith(u8, field.name, "deprecated_")) continue;
        if (comptime std.mem.eql(u8, field.name, "reserved")) continue;
        if (comptime std.mem.eql(u8, field.name, "opaque")) continue;

        buffer.print("  DEFINE_ACCESSORS_METHODS({s}, {s}, {s})\n", .{
            tb_struct,
            field.name,
            ruby_class,
        });
    }
}

pub fn main() !void {
    @setEvalBranchQuota(100_000);

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    var buffer = Buffer.init(allocator);
    buffer.print(
        \\//////////////////////////////////////////////////////////////
        \\// This file was auto-generated by ruby_native_bindings.zig //
        \\//              Do not manually modify.                     //
        \\//////////////////////////////////////////////////////////////
        \\
        \\#include <ruby.h>
        \\#include "tb_client.h"
        \\#include "tb_macros.h"
        \\
        \\void tb_define_enums_and_bitmasks(VALUE module) {{
        \\
        \\
    , .{});

    // Emit enum and bitmask declarations.
    inline for (mappings_all) |type_mapping| {
        const ZigType = type_mapping[0];
        const ruby_name = type_mapping[1];

        switch (@typeInfo(ZigType)) {
            .Struct => |info| switch (info.layout) {
                .auto => @compileError("Invalid C struct type: " ++ @typeName(ZigType)),
                .@"packed" => try emit_enum(&buffer, ZigType, info, ruby_name, &.{"padding"}),
                .@"extern" => continue,
            },
            .Enum => |info| {
                comptime var skip: []const []const u8 = &.{};
                if (ZigType == exports.tb_operation) {
                    skip = &.{ "reserved", "root", "register" };
                }

                try emit_enum(&buffer, ZigType, info, ruby_name, skip);
            },
            else => @compileError("Invalid C struct type: " ++ @typeName(ZigType)),
        }
    }
    buffer.print("}}\n\n", .{});

    // define the struc
    inline for (mappings_all) |type_mapping| {
        const ZigType = type_mapping[0];
        const ruby_name = type_mapping[1];

        switch (@typeInfo(ZigType)) {
            .Struct => |info| switch (info.layout) {
                .auto => @compileError("Invalid C struct type: " ++ @typeName(ZigType)),
                .@"packed" => continue,
                .@"extern" => {
                    const tb_struct = if (type_mapping.len > 2) type_mapping[2] else @compileError("Missing tb_struct name");
                    try emit_struct_native_extensions(&buffer, info, ruby_name, tb_struct);
                },
            },
            else => continue,
        }
    }

    // define ruby accessors
    buffer.print(
        \\//Ruby method accessors
        \\void tb_define_ruby_accessors(VALUE module) {{
        \\
    , .{});
    inline for (mappings_all) |type_mapping| {
        const ZigType = type_mapping[0];
        const ruby_name = type_mapping[1];

        switch (@typeInfo(ZigType)) {
            .Struct => |info| switch (info.layout) {
                .auto => @compileError("Invalid C struct type: " ++ @typeName(ZigType)),
                .@"packed" => continue,
                .@"extern" => {
                    const tb_struct = if (type_mapping.len > 2) type_mapping[2] else @compileError("Missing tb_struct name");
                    try emit_ruby_method_accessors(&buffer, info, ruby_name, tb_struct);
                },
            },
            else => continue,
        }
    }
    buffer.print("}}\n", .{});
    try std.io.getStdOut().writeAll(buffer.inner.items);
}
