// bridge.c - Consistent naming: Ruby function -> zig_ruby_function
#include "ruby.h"

// ========================================================================
// CONSTANTS - Ruby macros exported as zig_MACRO_NAME
// ========================================================================
const VALUE zig_Qnil = Qnil;
const VALUE zig_Qtrue = Qtrue;
const VALUE zig_Qfalse = Qfalse;
const int zig_RUBY_TYPED_FREE_IMMEDIATELY = RUBY_TYPED_FREE_IMMEDIATELY;
void (*zig_RUBY_DEFAULT_FREE)(void*) = RUBY_DEFAULT_FREE;

// Ruby type constants
const int zig_T_FIXNUM = T_FIXNUM;
const int zig_T_STRING = T_STRING;
const int zig_T_ARRAY = T_ARRAY;
const int zig_T_HASH = T_HASH;
const int zig_T_NIL = T_NIL;
const int zig_T_TRUE = T_TRUE;
const int zig_T_FALSE = T_FALSE;

// Ruby class constants
const VALUE zig_rb_cObject = rb_cObject;
const VALUE zig_rb_eStandardError = rb_eStandardError;
const VALUE zig_rb_eRuntimeError = rb_eRuntimeError;

// ========================================================================
// TYPEDDATA FUNCTIONS - rb_* -> zig_rb_*
// ========================================================================
VALUE zig_rb_data_typed_object_alloc(VALUE klass, const rb_data_type_t* data_type) {
    return rb_data_typed_object_alloc(klass, NULL, data_type);
}

void* zig_rb_check_typeddata(VALUE obj, const rb_data_type_t* data_type) {
    return rb_check_typeddata(obj, data_type);
}

// ========================================================================
// MACRO WRAPPERS - MACRO_NAME -> zig_MACRO_NAME
// ========================================================================
void* zig_RTYPEDDATA_DATA(VALUE obj) {
    return RTYPEDDATA_DATA(obj);
}

int zig_NIL_P(VALUE obj) {
    return NIL_P(obj);
}

int zig_RTEST(VALUE obj) {
    return RTEST(obj);
}

int zig_FIXNUM_P(VALUE obj) {
    return FIXNUM_P(obj);
}

int zig_TYPE(VALUE obj) {
    return TYPE(obj);
}

// ========================================================================
// NUMERIC CONVERSIONS - MACRO_NAME -> zig_MACRO_NAME
// ========================================================================
long zig_FIX2LONG(VALUE obj) {
    return FIX2LONG(obj);
}

VALUE zig_LONG2FIX(long val) {
    return LONG2FIX(val);
}

int zig_FIX2INT(VALUE obj) {
    return FIX2INT(obj);
}

VALUE zig_INT2FIX(int val) {
    return INT2FIX(val);
}

int zig_NUM2INT(VALUE obj) {
    return NUM2INT(obj);
}

VALUE zig_INT2NUM(int val) {
    return INT2NUM(val);
}

long zig_NUM2LONG(VALUE obj) {
    return NUM2LONG(obj);
}

VALUE zig_LONG2NUM(long val) {
    return LONG2NUM(val);
}

double zig_NUM2DBL(VALUE obj) {
    return NUM2DBL(obj);
}

VALUE zig_DBL2NUM(double val) {
    return DBL2NUM(val);
}

// ========================================================================
// CLASS DEFINITION FUNCTIONS - rb_* -> zig_rb_*
// ========================================================================
VALUE zig_rb_define_module(const char* name) {
    return rb_define_module(name);
}

VALUE zig_rb_define_class(const char* name, VALUE super) {
    return rb_define_class(name, super);
}

VALUE zig_rb_define_class_under(VALUE outer, const char* name, VALUE super) {
    return rb_define_class_under(outer, name, super);
}

void zig_rb_define_alloc_func(VALUE klass, VALUE (*func)(VALUE)) {
    rb_define_alloc_func(klass, func);
}

void zig_rb_define_method(VALUE klass, const char* name, VALUE (*func)(), int argc) {
    rb_define_method(klass, name, func, argc);
}

void zig_rb_define_singleton_method(VALUE obj, const char* name, VALUE (*func)(), int argc) {
    rb_define_singleton_method(obj, name, func, argc);
}

// ========================================================================
// ARGUMENT SCANNING - rb_* -> zig_rb_*
// ========================================================================
int zig_rb_scan_args(int argc, const VALUE* argv, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = rb_scan_args(argc, argv, fmt, args);
    va_end
