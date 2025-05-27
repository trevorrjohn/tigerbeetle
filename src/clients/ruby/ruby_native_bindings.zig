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

fn to_uppercase(comptime input: []const u8) [input.len]u8 {
    comptime var output: [input.len]u8 = undefined;
    inline for (&output, 0..) |*char, i| {
        char.* = input[i];
        char.* -= 32 * @as(u8, @intFromBool(char.* >= 'a' and char.* <= 'z'));
    }
    return output;
}

fn emit_rb_class_for_struct(buffer: *Buffer, comptime ctype: []const u8) void {
    const tb_struct_t = ctype ++ "_t";
    const rb_prefix = "rb_" ++ ctype;
    const rb_struct_type = rb_prefix ++ "_type";
    const rb_size_fn = rb_prefix ++ "_size";
    buffer.print(
        \\static size_t {s}(const void *ptr) {{
        \\  return sizeof({s});
        \\}}
        \\
        \\
    , .{ rb_size_fn, tb_struct_t });

    buffer.print(
        \\const rb_data_type_t {s} = {{
        \\  .wrap_struct_name = "{s}",
        \\  .function = {{
        \\    .dmark = NULL,
        \\    .dfree = RUBY_DEFAULT_FREE,
        \\    .dsize = {s},
        \\  }},
        \\  .data = NULL,
        \\  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
        \\}};
        \\
        \\
    , .{ rb_struct_type, tb_struct_t, rb_size_fn });

    buffer.print(
        \\static VALUE {s}_alloc(VALUE self) {{
        \\  {s} *ptr;
        \\  return TypedData_Make_Struct(self, {s}, &{s}, ptr);
        \\}}
        \\
        \\
    , .{ rb_prefix, tb_struct_t, tb_struct_t, rb_struct_type });

    buffer.print(
        \\static VALUE {s}_initialize(int argc, VALUE *argv, VALUE self) {{
        \\  VALUE kwargs;
        \\  rb_scan_args(argc, argv, ":", &kwargs);
        \\
        \\  if (NIL_P(kwargs)) {{
        \\    return self;
        \\  }}
        \\
        \\  {s} *wrapper;
        \\  TypedData_Get_Struct(self, {s}, &{s}, wrapper);
        \\
        \\  VALUE keys = rb_funcall(kwargs, rb_intern("keys"), 0);
        \\  long keys_len = RARRAY_LEN(keys);
        \\
        \\  for (long i = 0; i < keys_len; i++) {{
        \\      VALUE key = RARRAY_AREF(keys, i);
        \\      VALUE value = rb_hash_aref(kwargs, key);
        \\      VALUE key_str = rb_funcall(key, rb_intern("to_s"), 0);
        \\      const char *key_cstr = StringValueCStr(key_str);
        \\      size_t setter_len = strlen(key_cstr) + 2; // +1 for '=', +1 for '\0'
        \\      char *setter_name = ALLOCA_N(char, setter_len);
        \\      snprintf(setter_name, setter_len, "%s=", key_cstr);
        \\      ID setter_id = rb_intern(setter_name);
        \\      if (!rb_respond_to(self, setter_id)) {{
        \\        rb_raise(rb_eNoMethodError, "undefined method '%s' for object", setter_name);
        \\      }}
        \\
        \\      rb_funcall(self, setter_id, 1, value);
        \\  }}
        \\  return self;
        \\}}
        \\
        \\
    , .{ rb_prefix, tb_struct_t, tb_struct_t, rb_struct_type });
}

fn emit_uint128_rb_accessors(buffer: *Buffer, comptime ctype: []const u8, comptime field_name: []const u8) void {
    const tb_struct_t = ctype ++ "_t";
    const rb_prefix = "rb_" ++ ctype;
    const rb_struct_type = rb_prefix ++ "_type";

    buffer.print(
        \\static VALUE {s}_get_{s}(VALUE self) {{
        \\  {s} *obj;
        \\  TypedData_Get_Struct(self, {s}, &{s}, obj);
        \\  return uint128_to_hex_string(obj->{s});
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, tb_struct_t, tb_struct_t, rb_struct_type, field_name });

    buffer.print(
        \\static VALUE {s}_set_{s}(VALUE self, VALUE val) {{
        \\  {s} *obj;
        \\  TypedData_Get_Struct(self, {s}, &{s}, obj);
        \\  obj->{s} = hex_string_to_uint128(val);
        \\  return val;
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, tb_struct_t, tb_struct_t, rb_struct_type, field_name });
}

fn emit_uint_rb_accessors(buffer: *Buffer, comptime ctype: []const u8, comptime field_name: []const u8, comptime uint_size: u8) void {
    const tb_struct_t = ctype ++ "_t";
    const rb_prefix = "rb_" ++ ctype;
    const rb_struct_type = rb_prefix ++ "_type";
    const uint_type = switch (uint_size) {
        8 => "uint8_t",
        16 => "uint16_t",
        32 => "uint32_t",
        64 => "uint64_t",
        else => @compileError("Invalid uint type: " ++ field_name),
    };

    buffer.print(
        \\static VALUE {s}_get_{s}(VALUE self) {{
        \\  {s} *obj;
        \\  TypedData_Get_Struct(self, {s}, &{s}, obj);
        \\  return ULL2NUM(obj->{s});
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, tb_struct_t, tb_struct_t, rb_struct_type, field_name });

    // TODO optimize this based on uint_type?
    buffer.print(
        \\static VALUE {s}_set_{s}(VALUE self, VALUE val) {{
        \\  {s} *obj;
        \\  TypedData_Get_Struct(self, {s}, &{s}, obj);
        \\  obj->{s} = ({s})NUM2ULL(val);
        \\  return val;
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, tb_struct_t, tb_struct_t, rb_struct_type, field_name, uint_type });
}

fn emit_void_ptr_rb_accessors(buffer: *Buffer, comptime ctype: []const u8, comptime field_name: []const u8) void {
    const tb_struct_t = ctype ++ "_t";
    const rb_prefix = "rb_" ++ ctype;
    const rb_struct_type = rb_prefix ++ "_type";

    buffer.print(
        \\static VALUE {s}_get_{s}(VALUE self) {{
        \\  rb_raise(rb_eRuntimeError, "Cannot access void* field - {s}");
        \\  return Qnil;
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, field_name });

    buffer.print(
        \\static VALUE {s}_set_{s}(VALUE self, VALUE val) {{
        \\  if (NIL_P(val)) {{
        \\    rb_raise(rb_eTypeError, "Expected a non nil value");
        \\    return Qnil;
        \\  }}
        \\  if (!RB_TYPE_P(val, T_STRING)) {{
        \\    rb_raise(rb_eTypeError, "Expected a straing");
        \\  }}
        \\  {s} *obj;
        \\  TypedData_Get_Struct(self, {s}, &{s}, obj);
        \\  if (obj->{s} != NULL) {{
        \\    rb_raise(rb_eRuntimeError, "Void* field is already set");
        \\    return Qnil;
        \\  }}
        \\  size_t data_size = (size_t)RSTRING_LEN(val);
        \\  char *data = (char*)malloc(data_size);
        \\  memcpy(data, RSTRING_PTR(val), data_size);
        \\  obj->{s} = data;
        \\  return val;
        \\}}
        \\
        \\
    , .{ rb_prefix, field_name, tb_struct_t, tb_struct_t, rb_struct_type, field_name, field_name });
}

fn emit_rb_define_accessors(buffer: *Buffer, comptime class_name: []const u8, comptime ctype: []const u8, comptime field_name: []const u8) void {
    const rb_prefix = "rb_" ++ ctype;

    buffer.print(
        \\  rb_define_method({s}, "{s}",  {s}_get_{s}, 0);
        \\  rb_define_method({s}, "{s}=",  {s}_set_{s}, 1);
        \\
    , .{ class_name, field_name, rb_prefix, field_name, class_name, field_name, rb_prefix, field_name });
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
        .Array => @compileError("Invalid C array type: " ++ @typeName(Type)),
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
    buffer.print("// Definitions: {s} - {s}_t\n", .{ ruby_name, tb_struct });
    emit_rb_class_for_struct(buffer, tb_struct);
    inline for (type_info.fields) |field| {
        if (comptime std.mem.startsWith(u8, field.name, "deprecated_")) continue;
        if (comptime std.mem.eql(u8, field.name, "reserved")) continue;
        if (comptime std.mem.eql(u8, field.name, "opaque")) continue;

        switch (@typeInfo(field.type)) {
            .Enum => |info| {
                const tag_type = info.tag_type;
                switch (@typeInfo(tag_type)) {
                    .Int => |arr_info| {
                        emit_uint_rb_accessors(buffer, tb_struct, field.name, arr_info.bits);
                    },
                    else => @compileError("Unsupported enum type: " ++ @typeName(tag_type)),
                }
            },
            .Int => |info| {
                assert(info.signedness == .unsigned);

                const ctype = comptime zig_type_to_ctype(field.type);
                if (comptime std.mem.eql(u8, "tb_uint128_t", ctype)) {
                    emit_uint128_rb_accessors(buffer, tb_struct, field.name);
                } else {
                    emit_uint_rb_accessors(buffer, tb_struct, field.name, info.bits);
                }
            },
            .Optional => |info| switch (@typeInfo(info.child)) {
                .Pointer => {
                    emit_void_ptr_rb_accessors(buffer, tb_struct, field.name);
                },
                else => @compileError("Unsupported optional type: " ++ @typeName(field.type)),
            },
            .Struct => |info| switch (info.layout) {
                .auto => @compileError("Invalid struct type: " ++ @typeName(field.type)),
                .@"packed" => {
                    const backing_int = info.backing_integer.?;
                    const bits = @typeInfo(backing_int).Int.bits;
                    emit_uint_rb_accessors(buffer, tb_struct, field.name, bits);
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
        emit_rb_define_accessors(buffer, ruby_class, tb_struct, field.name);
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
        \\#include "tb_helpers.h"
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

    // first let's output all the struct method definitions
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
