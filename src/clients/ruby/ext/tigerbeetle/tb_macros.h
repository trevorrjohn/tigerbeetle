// tb_macros.h
#ifndef TB_MACROS_H
#define TB_MACROS_H

#include <ruby.h>
#include <stdint.h>
#include <inttypes.h> // For PRIx64

// Helper function declarations for uint128_t conversion
VALUE uint128_to_hex_string(tb_uint128_t value);
tb_uint128_t hex_string_to_uint128(VALUE str);

#define DEFINE_RB_CLASS_FOR_STRUCT(type, class_name) \
  static size_t tb_##type##_size(const void *ptr) { \
      return sizeof(type##_t); \
  } \
  \
  static const rb_data_type_t type##_type = { \
      .wrap_struct_name = #class_name, \
      .function = { \
          .dmark = NULL, \
          .dfree = RUBY_DEFAULT_FREE, \
          .dsize = type##_size, \
      }, \
      .data = NULL, \
      .flags = RUBY_TYPED_FREE_IMMEDIATELY, \
  }; \
  \
  static VALUE tb_##type##_aloc(VALUE self) { \
    type##_t *obj; \
    return TypedData_Make_Struct(self, type##_t, &type##_type, obj); \
  } \

#define DEFINE_UINT128_ACCESSORS(type, field_name) \
  static VALUE rb_##type##_get_##field_name(VALUE self) { \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    return uint128_to_hex_string(obj->field_name); \
  } \
  \
  static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    obj->field_name = hex_string_to_uint128(val); \
    return val; \
  } \

#define DEFINE_ACCESSORS_METHODS(klass, type, field_name) \
  rb_define_method(klass, #field_name, rb_##type##_get_##field_name, 0); \
  rb_define_method(klass, #field_name "=", rb_##type##_set_##field_name, 1); \

#define DEFINE_UINT_ACCESSORS(type, ctype, field_name) \
  static VALUE rb_##type##_get_##field_name(VALUE self) { \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    return (#ctype)ULL2NUM(obj->field_name); \
  } \
  \
  static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
    Check_Type(val, T_FIXNUM); \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    obj->field_name = (ctype)NUM2ULL(val); \
    return val; \
  } \

#define DEFINE_BYTE_ARRAY_ACCESSORS(type, field_name, ctype, array_size) \
  static VALUE rb_##type##_get_##field_name(VALUE self) { \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    long data_size = (long)sizeof(ctype) * array_size; \
    return rb_str_new((const char *)obj->field_name, data_size); \
  } \
  \
  static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
    Check_Type(val, T_STRING); \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    size_t max_size = (size_t)sizeof(ctype) * array_size; \
    size_t data_size = RSTRING_LEN(val); \
    if (data_size > max_size) { \
      rb_raise(rb_eArgError, "Data too long for field %s max size %d", #field_name, max_size); \
    } \
    memset(obj->field_name, 0, max_size); /* Clear existing data */ \
    memcpy(obj->field_name, RSTRING_PTR(val), data_size); \
    return val; \
  } \

// Do not define getter for a void* once it is set, no going back
#define DEFINE_VOID_PTR_ACCESSORS(type, field_name) \
  static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
    if (NIL_P(val)) { \
      rb_raise(rb_eTypeError, "Expected a non-nil value for field field_name"); \
    } \
    if (RB_TYPE_P(val, T_STRING)) {\
      rb_raise(rb_eTypeError, "Expected a string for field field_name"); \
    } \
    type##_t *obj; \
    TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
    if (obj->field_name != NULL) { \
      rb_raise(rb_eArgError, "Field field_name already set"); \
    } \
    \
    size_t data_size = (size_t)RSTRING_LEN(val); \
    char* val = (char*)ZALLOC(data_size); \
    memcpy(val, RSTRING_PTR(val), data_size); \
    obj->field_name = (void *)val; \
    return val; \
  } \


// Helper function implementations
VALUE uint128_to_hex_string(tb_uint128_t value) {
  if (value == NULL) {
    // TODO should this raise an error
    return UINT2NUM(0);
  }

  char buffer[33]; // 32 hex digits + null terminator
  uint64_t high = (uint64_t)(value >> 64);
  uint64_t low = (uint64_t)value;

  snprintf(buffer, sizeof(buffer), "%016" PRIx64 "%016" PRIx64, high, low);
  return rb_str_new_cstr(buffer);
}

tb_uint128_t hex_string_to_uint128(VALUE str) {
  if (NIL_P(str)) {
    // TODO should this raise an error
    return 0;
  }

  Check_Type(str, T_STRING);

  char *hex_str = StringValuePtr(str);
  tb_uint128_t result = 0;

  // Parse the hex string to 128-bit integer
  for (size_t i = 0; i < RSTRING_LEN(str); i++) {
    char c = hex_str[i];
    int val = 0;

    if (c >= '0' && c <= '9') val = c - '0';
    else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
    else continue; // Skip non-hex characters

    result = (result << 4) | val;
  }

  return result;
}

#endif

