// ruby_bridge.zig - Consistent bridge interface (mirrors bridge.c exactly)

const std = @import("std");

// Ruby types
pub const VALUE = c_ulong;
pub const ID = c_ulong;

pub const rb_data_type_t = extern struct {
    wrap_struct_name: [*c]const u8,
    function: extern struct {
        dmark: ?*const fn (?*const anyopaque) callconv(.C) void,
        dfree: ?*const fn (?*anyopaque) callconv(.C) void,
        dsize: ?*const fn (?*const anyopaque) callconv(.C) usize,
    },
    data: ?*anyopaque,
    flags: c_int,
};

// ========================================================================
// CONSTANTS - Mirror every zig_* constant from bridge.c
// ========================================================================
extern const zig_Qnil: VALUE;
extern const zig_Qtrue: VALUE;
extern const zig_Qfalse: VALUE;
extern const zig_RUBY_TYPED_FREE_IMMEDIATELY: c_int;
extern const zig_RUBY_DEFAULT_FREE: ?*const fn (?*anyopaque) callconv(.C) void;

// Ruby type constants
extern const zig_T_FIXNUM: c_int;
extern const zig_T_STRING: c_int;
extern const zig_T_ARRAY: c_int;
extern const zig_T_HASH: c_int;
extern const zig_T_NIL: c_int;
extern const zig_T_TRUE: c_int;
extern const zig_T_FALSE: c_int;

// Ruby class constants
extern const zig_rb_cObject: VALUE;
extern const zig_rb_eStandardError: VALUE;
extern const zig_rb_eRuntimeError: VALUE;

// ========================================================================
// TYPEDDATA FUNCTIONS - Mirror every zig_rb_* function from bridge.c
// ========================================================================
extern fn zig_rb_data_typed_object_alloc(klass: VALUE, data_type: *const rb_data_type_t) VALUE;
extern fn zig_rb_check_typeddata(obj: VALUE, data_type: *const rb_data_type_t) ?*anyopaque;

// ========================================================================
// MACRO WRAPPERS - Mirror every zig_MACRO_* function from bridge.c
// ========================================================================
extern fn zig_RTYPEDDATA_DATA(obj: VALUE) ?*anyopaque;
extern fn zig_NIL_P(obj: VALUE) c_int;
extern fn zig_RTEST(obj: VALUE) c_int;
extern fn zig_FIXNUM_P(obj: VALUE) c_int;
extern fn zig_TYPE(obj: VALUE) c_int;

// ========================================================================
// NUMERIC CONVERSIONS - Mirror every zig_* conversion from bridge.c
// ========================================================================
extern fn zig_FIX2LONG(obj: VALUE) c_long;
extern fn zig_LONG2FIX(val: c_long) VALUE;
extern fn zig_FIX2INT(obj: VALUE) c_int;
extern fn zig_INT2FIX(val: c_int) VALUE;
extern fn zig_NUM2INT(obj: VALUE) c_int;
extern fn zig_INT2NUM(val: c_int) VALUE;
extern fn zig_NUM2LONG(obj: VALUE) c_long;
extern fn zig_LONG2NUM(val: c_long) VALUE;
extern fn zig_NUM2DBL(obj: VALUE) f64;
extern fn zig_DBL2NUM(val: f64) VALUE;

// ========================================================================
// CLASS DEFINITION FUNCTIONS - Mirror every zig_rb_* function from bridge.c
// ========================================================================
extern fn zig_rb_define_module(name: [*c]const u8) VALUE;
extern fn zig_rb_define_class(name: [*c]const u8, super: VALUE) VALUE;
extern fn zig_rb_define_class_under(outer: VALUE, name: [*c]const u8, super: VALUE) VALUE;
extern fn zig_rb_define_alloc_func(klass: VALUE, func: *const fn (VALUE) callconv(.C) VALUE) void;
extern fn zig_rb_define_method(klass: VALUE, name: [*c]const u8, func: *const fn () callconv(.C) VALUE, argc: c_int) void;
extern fn zig_rb_define_singleton_method(obj: VALUE, name: [*c]const u8, func: *const fn () callconv(.C) VALUE, argc: c_int) void;

// ========================================================================
// ARGUMENT SCANNING - Mirror zig_rb_* functions from bridge.c
// ========================================================================
extern fn zig_rb_scan_args(argc: c_int, argv: [*c]VALUE, fmt: [*c]const u8, ...) c_int;

// ========================================================================
// STRING FUNCTIONS - Mirror every zig_rb_* and zig_R* function from bridge.c
// ========================================================================
extern fn zig_rb_str_new(ptr: [*c]const u8, len: c_long) VALUE;
extern fn zig_rb_str_new_cstr(ptr: [*c]const u8) VALUE;
extern fn zig_RSTRING_PTR(str: VALUE) [*c]u8;
extern fn zig_RSTRING_LEN(str: VALUE) c_long;
extern fn zig_rb_str_to_str(str: VALUE) VALUE;

// ========================================================================
// ARRAY FUNCTIONS - Mirror every zig_rb_* and zig_R* function from bridge.c
// ========================================================================
extern fn zig_rb_ary_new() VALUE;
extern fn zig_rb_ary_new_capa(capa: c_long) VALUE;
extern fn zig_rb_ary_push(ary: VALUE, item: VALUE) VALUE;
extern fn zig_rb_ary_entry(ary: VALUE, index: c_long) VALUE;
extern fn zig_RARRAY_LEN(ary: VALUE) c_long;
extern fn zig_RARRAY_PTR(ary: VALUE) [*c]VALUE;

// ========================================================================
// HASH FUNCTIONS - Mirror every zig_rb_* function from bridge.c
// ========================================================================
extern fn zig_rb_hash_new() VALUE;
extern fn zig_rb_hash_aref(hash: VALUE, key: VALUE) VALUE;
extern fn zig_rb_hash_aset(hash: VALUE, key: VALUE, val: VALUE) VALUE;

// ========================================================================
// SYMBOL FUNCTIONS - Mirror every zig_rb_* and zig_* function from bridge.c
// ========================================================================
extern fn zig_rb_intern(name: [*c]const u8) ID;
extern fn zig_ID2SYM(id: ID) VALUE;
extern fn zig_SYM2ID(sym: VALUE) ID;

// ========================================================================
// ERROR HANDLING - Mirror every zig_rb_* function from bridge.c
// ========================================================================
extern fn zig_rb_raise(exception: VALUE, fmt: [*c]const u8, ...) void;
extern fn zig_rb_warn(fmt: [*c]const u8, ...) void;

// ========================================================================
// OBJECT INSPECTION - Mirror every zig_rb_* function from bridge.c
// ========================================================================
extern fn zig_rb_inspect(obj: VALUE) VALUE;
extern fn zig_rb_p(obj: VALUE) void;
extern fn zig_rb_obj_classname(obj: VALUE) [*c]const u8;
extern fn zig_rb_obj_class(obj: VALUE) VALUE;

// ========================================================================
// MEMORY MANAGEMENT - Mirror every zig_ruby_* function from bridge.c
// ========================================================================
extern fn zig_ruby_xmalloc(size: usize) ?*anyopaque;
extern fn zig_ruby_xfree(ptr: ?*anyopaque) void;

// ========================================================================
// RBBRIDGE - Ruby method names kept exactly as-is
// ========================================================================
pub const RbBridge = struct {
    // Constants (direct access to bridge constants)
    pub const Qnil = zig_Qnil;
    pub const Qtrue = zig_Qtrue;
    pub const Qfalse = zig_Qfalse;
    pub const TYPED_FREE_IMMEDIATELY = zig_RUBY_TYPED_FREE_IMMEDIATELY;
    pub const DEFAULT_FREE = zig_RUBY_DEFAULT_FREE;
    pub const T_FIXNUM = zig_T_FIXNUM;
    pub const T_STRING = zig_T_STRING;
    pub const T_ARRAY = zig_T_ARRAY;
    pub const T_HASH = zig_T_HASH;
    pub const T_NIL = zig_T_NIL;
    pub const rb_cObject = zig_rb_cObject;
    pub const rb_eStandardError = zig_rb_eStandardError;

    // ========================================================================
    // TYPEDDATA OPERATIONS - Keep full rb_* names
    // ========================================================================
    pub fn rb_data_typed_object_alloc(klass: VALUE, data_type: *const rb_data_type_t) VALUE {
        return zig_rb_data_typed_object_alloc(klass, data_type);
    }

    pub fn rb_check_typeddata(obj: VALUE, data_type: *const rb_data_type_t) ?*anyopaque {
        return zig_rb_check_typeddata(obj, data_type);
    }

    // ========================================================================
    // MACRO OPERATIONS - Keep exact MACRO names
    // ========================================================================
    pub fn RTYPEDDATA_DATA(obj: VALUE) ?*anyopaque {
        return zig_RTYPEDDATA_DATA(obj);
    }

    pub fn NIL_P(obj: VALUE) bool {
        return zig_NIL_P(obj) != 0;
    }

    pub fn RTEST(obj: VALUE) bool {
        return zig_RTEST(obj) != 0;
    }

    pub fn FIXNUM_P(obj: VALUE) bool {
        return zig_FIXNUM_P(obj) != 0;
    }

    pub fn TYPE(obj: VALUE) c_int {
        return zig_TYPE(obj);
    }

    // ========================================================================
    // NUMERIC CONVERSIONS - Keep exact MACRO names
    // ========================================================================
    pub fn FIX2LONG(obj: VALUE) c_long {
        return zig_FIX2LONG(obj);
    }

    pub fn LONG2FIX(val: c_long) VALUE {
        return zig_LONG2FIX(val);
    }

    pub fn FIX2INT(obj: VALUE) c_int {
        return zig_FIX2INT(obj);
    }

    pub fn INT2FIX(val: c_int) VALUE {
        return zig_INT2FIX(val);
    }

    pub fn NUM2INT(obj: VALUE) c_int {
        return zig_NUM2INT(obj);
    }

    pub fn INT2NUM(val: c_int) VALUE {
        return zig_INT2NUM(val);
    }

    pub fn NUM2LONG(obj: VALUE) c_long {
        return zig_NUM2LONG(obj);
    }

    pub fn LONG2NUM(val: c_long) VALUE {
        return zig_LONG2NUM(val);
    }

    pub fn NUM2DBL(obj: VALUE) f64 {
        return zig_NUM2DBL(obj);
    }

    pub fn DBL2NUM(val: f64) VALUE {
        return zig_DBL2NUM(val);
    }

    // ========================================================================
    // CLASS DEFINITION - Keep full rb_* names
    // ========================================================================
    pub fn rb_define_module(name: [*c]const u8) VALUE {
        return zig_rb_define_module(name);
    }

    pub fn rb_define_class(name: [*c]const u8, super: VALUE) VALUE {
        return zig_rb_define_class(name, super);
    }

    pub fn rb_define_class_under(outer: VALUE, name: [*c]const u8, super: VALUE) VALUE {
        return zig_rb_define_class_under(outer, name, super);
    }

    pub fn rb_define_alloc_func(klass: VALUE, func: *const fn (VALUE) callconv(.C) VALUE) void {
        zig_rb_define_alloc_func(klass, func);
    }

    pub fn rb_define_method(klass: VALUE, name: [*c]const u8, func: *const fn () callconv(.C) VALUE, argc: c_int) void {
        zig_rb_define_method(klass, name, func, argc);
    }

    pub fn rb_define_singleton_method(obj: VALUE, name: [*c]const u8, func: *const fn () callconv(.C) VALUE, argc: c_int) void {
        zig_rb_define_singleton_method(obj, name, func, argc);
    }

    // ========================================================================
    // ARGUMENT SCANNING - Keep full rb_* names
    // ========================================================================
    pub fn rb_scan_args(argc: c_int, argv: [*c]VALUE, fmt: [*c]const u8, args: anytype) c_int {
        return @call(.auto, zig_rb_scan_args, .{ argc, argv, fmt } ++ args);
    }

    // ========================================================================
    // STRING OPERATIONS - Keep full rb_* and MACRO names
    // ========================================================================
    pub fn rb_str_new(ptr: [*c]const u8, len: c_long) VALUE {
        return zig_rb_str_new(ptr, len);
    }

    pub fn rb_str_new_cstr(ptr: [*c]const u8) VALUE {
        return zig_rb_str_new_cstr(ptr);
    }

    pub fn RSTRING_PTR(str: VALUE) [*c]u8 {
        return zig_RSTRING_PTR(str);
    }

    pub fn RSTRING_LEN(str: VALUE) c_long {
        return zig_RSTRING_LEN(str);
    }

    pub fn rb_str_to_str(str: VALUE) VALUE {
        return zig_rb_str_to_str(str);
    }

    // ========================================================================
    // ARRAY OPERATIONS - Keep full rb_* and MACRO names
    // ========================================================================
    pub fn rb_ary_new() VALUE {
        return zig_rb_ary_new();
    }

    pub fn rb_ary_new_capa(capa: c_long) VALUE {
        return zig_rb_ary_new_capa(capa);
    }

    pub fn rb_ary_push(ary: VALUE, item: VALUE) VALUE {
        return zig_rb_ary_push(ary, item);
    }

    pub fn rb_ary_entry(ary: VALUE, index: c_long) VALUE {
        return zig_rb_ary_entry(ary, index);
    }

    pub fn RARRAY_LEN(ary: VALUE) c_long {
        return zig_RARRAY_LEN(ary);
    }

    pub fn RARRAY_PTR(ary: VALUE) [*c]VALUE {
        return zig_RARRAY_PTR(ary);
    }

    // ========================================================================
    // HASH OPERATIONS - Keep full rb_* names
    // ========================================================================
    pub fn rb_hash_new() VALUE {
        return zig_rb_hash_new();
    }

    pub fn rb_hash_aref(hash: VALUE, key: VALUE) VALUE {
        return zig_rb_hash_aref(hash, key);
    }

    pub fn rb_hash_aset(hash: VALUE, key: VALUE, val: VALUE) VALUE {
        return zig_rb_hash_aset(hash, key, val);
    }

    // ========================================================================
    // SYMBOL OPERATIONS - Keep full rb_* and MACRO names
    // ========================================================================
    pub fn rb_intern(name: [*c]const u8) ID {
        return zig_rb_intern(name);
    }

    pub fn ID2SYM(id: ID) VALUE {
        return zig_ID2SYM(id);
    }

    pub fn SYM2ID(sym: VALUE) ID {
        return zig_SYM2ID(sym);
    }

    // ========================================================================
    // ERROR HANDLING - Keep full rb_* names
    // ========================================================================
    pub fn rb_raise(exception: VALUE, fmt: [*c]const u8, args: anytype) void {
        @call(.auto, zig_rb_raise, .{ exception, fmt } ++ args);
    }

    pub fn rb_warn(fmt: [*c]const u8, args: anytype) void {
        @call(.auto, zig_rb_warn, .{fmt} ++ args);
    }

    // ========================================================================
    // OBJECT INSPECTION - Keep full rb_* names
    // ========================================================================
    pub fn rb_inspect(obj: VALUE) VALUE {
        return zig_rb_inspect(obj);
    }

    pub fn rb_p(obj: VALUE) void {
        zig_rb_p(obj);
    }

    pub fn rb_obj_classname(obj: VALUE) [*c]const u8 {
        return zig_rb_obj_classname(obj);
    }

    pub fn rb_obj_class(obj: VALUE) VALUE {
        return zig_rb_obj_class(obj);
    }

    // ========================================================================
    // MEMORY MANAGEMENT - Keep full ruby_* names
    // ========================================================================
    pub fn ruby_xmalloc(size: usize) ?*anyopaque {
        return zig_ruby_xmalloc(size);
    }

    pub fn ruby_xfree(ptr: ?*anyopaque) void {
        zig_ruby_xfree(ptr);
    }

    // ========================================================================
    // CONVENIENCE METHODS - Zig-friendly helpers (clearly marked as helpers)
    // ========================================================================

    // String helpers (clearly not Ruby methods - different naming)
    pub fn newString(data: []const u8) VALUE {
        return rb_str_new(data.ptr, @intCast(data.len));
    }

    pub fn stringSlice(str: VALUE) []const u8 {
        const ptr = RSTRING_PTR(str);
        const len = RSTRING_LEN(str);
        return ptr[0..@intCast(len)];
    }

    // Array helpers (clearly not Ruby methods - different naming)
    pub fn arrayLen(ary: VALUE) usize {
        return @intCast(RARRAY_LEN(ary));
    }

    pub fn arrayGet(ary: VALUE, index: usize) VALUE {
        return rb_ary_entry(ary, @intCast(index));
    }
};

// ========================================================================
// RBBRIDGE NAMING CONVENTION
// ========================================================================
//
// Ruby Function/Macro          Bridge Function              RbBridge Method
// ===================         ================             ===============
// rb_define_method         -> zig_rb_define_method      -> rb_define_method
// rb_str_new               -> zig_rb_str_new            -> rb_str_new
// rb_hash_aref             -> zig_rb_hash_aref          -> rb_hash_aref
// RSTRING_PTR              -> zig_RSTRING_PTR           -> RSTRING_PTR
// NIL_P                    -> zig_NIL_P                 -> NIL_P
// FIXNUM_P                 -> zig_FIXNUM_P              -> FIXNUM_P
// FIX2LONG                 -> zig_FIX2LONG              -> FIX2LONG
// Qnil                     -> zig_Qnil                  -> Qnil
// rb_cObject               -> zig_rb_cObject            -> rb_cObject
//
// RbBridge Method Naming Rules:
// 1. Ruby functions: keep EXACT name -> rb_define_method stays rb_define_method
// 2. Ruby macros: keep EXACT name -> NIL_P stays NIL_P, RSTRING_PTR stays RSTRING_PTR
// 3. Ruby constants: keep EXACT name -> rb_cObject stays rb_cObject, Qnil stays Qnil
//
// This makes RbBridge methods IDENTICAL to Ruby documentation:
// - Ruby docs: rb_str_new(ptr, len)
// - RbBridge: RbBridge.rb_str_new(ptr, len)
// - Ruby docs: RSTRING_PTR(str)
// - RbBridge: RbBridge.RSTRING_PTR(str)
// - Ruby docs: rb_define_method(klass, name, func, argc)
// - RbBridge: RbBridge.rb_define_method(klass, name, func, argc)
//
// Perfect 1:1 correspondence with Ruby C API!
