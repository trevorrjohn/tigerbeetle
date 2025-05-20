// tb_macros.h
#ifndef TB_MACROS_H
#define TB_MACROS_H

#include <ruby.h>
#include <stdint.h>
#include <inttypes.h> // For PRIx64

// Helper function declarations for uint128_t conversion
VALUE uint128_to_hex_string(tb_uint128_t value);
tb_uint128_t hex_string_to_uint128(VALUE str);

// // Macro for uint128_t fields (using hex string conversion)
// #define DEFINE_UINT128_ACCESSORS(klass, type, field_name) \
//     static VALUE rb_##type##_get_##field_name(VALUE self) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         return uint128_to_hex_string(obj->field_name); \
//     } \
//     static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         obj->field_name = hex_string_to_uint128(val); \
//         return val; \
//     } \
//     rb_define_method(klass, #field_name, rb_##type##_get_##field_name, 0); \
//     rb_define_method(klass, #field_name "=", rb_##type##_set_##field_name, 1);
//
// // Macro for uint64_t, uint32_t, uint16_t, uint8_t, size_t fields
// #define DEFINE_UINT_ACCESSORS(klass, type, field_name) \
//     static VALUE rb_##type##_get_##field_name(VALUE self) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         return ULL2NUM(obj->field_name); \
//     } \
//     static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         obj->field_name = NUM2ULL(val); \
//         return val; \
//     } \
//     rb_define_method(klass, #field_name, rb_##type##_get_##field_name, 0); \
//     rb_define_method(klass, #field_name "=", rb_##type##_set_##field_name, 1);
//
// // Macro for uint64_t, uint32_t, uint16_t, uint8_t, size_t fields
// #define DEFINE_BOOL_ACCESSORS(klass, type, field_name) \
//     static VALUE rb_##type##_get_##field_name(VALUE self) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         return obj->field_name ? Qtrue : Qfalse; \
//     } \
//     static VALUE rb_##type##_set_##field_name(VALUE self, VALUE val) { \
//         type##_t *obj; \
//         TypedData_Get_Struct(self, type##_t, &type##_type, obj); \
//         if (NIL_P(val)) { \
//           obj->field_name = NULL; \
//         } else { \
//           obj->field_name = RTEST(val); \
//         } \
//         return val; \
//     } \
//     rb_define_method(klass, #field_name, rb_##type##_get_##field_name, 0); \
//     rb_define_method(klass, #field_name "=", rb_##type##_set_##field_name, 1);
//
// // MACRO FOR DEFINING ALLOCATORS
// #DEFINE DEFINE_STRUCT_ALLOCATOR(KLASS, TYPE) \
//     STATIC VALUE RB_##TYPE##_ALLOC(VALUE KLASS) { \
//         TYPE##_T *OBJ = ALLOC(TYPE##_T); \
//         MEMSET(OBJ, 0, SIZEOF(TYPE##_T)); \
//         RETURN TYPEDDATA_WRAP_STRUCT(KLASS, &TYPE##_TYPE, OBJ); \
//     } \
//     RB_DEFINE_ALLOC_FUNC(KLASS, RB_##TYPE##_ALLOC);
//
// // MACRO TO DEFINE AN INITIALIZER METHOD
// #DEFINE DEFINE_STRUCT_INITIALIZER(TYPE, ...) \
//     STATIC VALUE RB_##TYPE##_INITIALIZE(INT ARGC, VALUE *ARGV, VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         \
//         IF (ARGC > 0) { \
//             VALUE HASH = ARGV[ARGC-1]; \
//             IF (RB_TYPE_P(HASH, T_HASH)) { \
//                 __VA_ARGS__ \
//             } \
//         } \
//         RETURN SELF; \
//     }
//
// // MACRO TO CHECK AND SET A HASH FIELD (UINT128_T VERSION)
// #DEFINE INIT_UINT128_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = HEX_STRING_TO_UINT128(VAL); \
//         } \
//     }
//
// // MACRO TO CHECK AND SET A HASH FIELD (UINT64, UINT32, UINT16, UINT8 VERSION)
// #DEFINE INIT_UINT_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = NUM2ULL(VAL); \
//         } \
//     }
//
// #DEFINE DEFINE_UINT128_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN UINT128_TO_HEX_STRING(OBJ->FIELD_NAME); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         OBJ->FIELD_NAME = HEX_STRING_TO_UINT128(VAL); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR UINT64_T FIELDS
// #DEFINE DEFINE_UINT64_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN ULL2NUM(OBJ->FIELD_NAME); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         OBJ->FIELD_NAME = NUM2ULL(VAL); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR UINT32_T FIELDS
// #DEFINE DEFINE_UINT32_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN UINT2NUM(OBJ->FIELD_NAME); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         OBJ->FIELD_NAME = NUM2UINT(VAL); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR UINT16_T FIELDS
// #DEFINE DEFINE_UINT16_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN UINT2NUM(OBJ->FIELD_NAME); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         OBJ->FIELD_NAME = NUM2UINT(VAL); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR UINT8_T FIELDS
// #DEFINE DEFINE_UINT8_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN INT2FIX(OBJ->FIELD_NAME); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         OBJ->FIELD_NAME = (UINT8_T)NUM2INT(VAL); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR VOID* POINTER FIELDS (STORE AS RUBY OBJECT)
// #DEFINE DEFINE_POINTER_ACCESSORS(KLASS, TYPE, FIELD_NAME) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         IF (OBJ->FIELD_NAME == NULL) RETURN QNIL; \
//         /* USE A PROPER APPROACH TO RETURN THE ASSOCIATED OBJECT */ \
//         /* FOR USER_DATA, RETURN THE STORED RUBY OBJECT */ \
//         RETURN (VALUE)OBJ->FIELD_NAME; \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         /* STORE RUBY OBJECT DIRECTLY, PROTECTED FROM GC BY ASSOCIATION WITH THIS OBJECT */ \
//         OBJ->FIELD_NAME = (VOID*)VAL; \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR DATA BUFFER FIELDS (VOID* WITH SIZE)
// #DEFINE DEFINE_DATA_BUFFER_ACCESSORS(KLASS, TYPE) \
//     STATIC VALUE RB_##TYPE##_GET_DATA(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         IF (OBJ->DATA == NULL || OBJ->DATA_SIZE == 0) RETURN QNIL; \
//         RETURN RB_STR_NEW(OBJ->DATA, OBJ->DATA_SIZE); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_DATA(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         CHECK_TYPE(VAL, T_STRING); \
//         /* FREE EXISTING DATA IF ANY */ \
//         IF (OBJ->DATA != NULL) { \
//             XFREE(OBJ->DATA); \
//         } \
//         /* ALLOCATE AND COPY NEW DATA */ \
//         OBJ->DATA_SIZE = RSTRING_LEN(VAL); \
//         OBJ->DATA = XMALLOC(OBJ->DATA_SIZE); \
//         MEMCPY(OBJ->DATA, RSTRING_PTR(VAL), OBJ->DATA_SIZE); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, "DATA", RB_##TYPE##_GET_DATA, 0); \
//     RB_DEFINE_METHOD(KLASS, "DATA=", RB_##TYPE##_SET_DATA, 1);
//
// // MACRO FOR BYTE ARRAY FIELDS
// #DEFINE DEFINE_BYTE_ARRAY_ACCESSORS(KLASS, TYPE, FIELD_NAME, ARRAY_SIZE) \
//     STATIC VALUE RB_##TYPE##_GET_##FIELD_NAME(VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         RETURN RB_STR_NEW((CONST CHAR*)OBJ->FIELD_NAME, ARRAY_SIZE); \
//     } \
//     STATIC VALUE RB_##TYPE##_SET_##FIELD_NAME(VALUE SELF, VALUE VAL) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         CHECK_TYPE(VAL, T_STRING); \
//         SIZE_T COPY_SIZE = RSTRING_LEN(VAL) < ARRAY_SIZE ? RSTRING_LEN(VAL) : ARRAY_SIZE; \
//         MEMSET(OBJ->FIELD_NAME, 0, ARRAY_SIZE); /* CLEAR EXISTING DATA */ \
//         MEMCPY(OBJ->FIELD_NAME, RSTRING_PTR(VAL), COPY_SIZE); \
//         RETURN VAL; \
//     } \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME, RB_##TYPE##_GET_##FIELD_NAME, 0); \
//     RB_DEFINE_METHOD(KLASS, #FIELD_NAME "=", RB_##TYPE##_SET_##FIELD_NAME, 1);
//
// // MACRO FOR DEFINING ALLOCATORS
// #DEFINE DEFINE_STRUCT_ALLOCATOR(KLASS, TYPE) \
//     STATIC VALUE RB_##TYPE##_ALLOC(VALUE KLASS) { \
//         TYPE##_T *OBJ = ALLOC(TYPE##_T); \
//         MEMSET(OBJ, 0, SIZEOF(TYPE##_T)); \
//         RETURN TYPEDDATA_WRAP_STRUCT(KLASS, &TYPE##_TYPE, OBJ); \
//     } \
//     RB_DEFINE_ALLOC_FUNC(KLASS, RB_##TYPE##_ALLOC);
//
// // MACRO TO DEFINE AN INITIALIZER METHOD
// #DEFINE DEFINE_STRUCT_INITIALIZER(TYPE, ...) \
//     STATIC VALUE RB_##TYPE##_INITIALIZE(INT ARGC, VALUE *ARGV, VALUE SELF) { \
//         TYPE##_T *OBJ; \
//         TYPEDDATA_GET_STRUCT(SELF, TYPE##_T, &TYPE##_TYPE, OBJ); \
//         \
//         IF (ARGC > 0) { \
//             VALUE HASH = ARGV[ARGC-1]; \
//             IF (RB_TYPE_P(HASH, T_HASH)) { \
//                 __VA_ARGS__ \
//             } \
//         } \
//         RETURN SELF; \
//     }
//
// // INITIALIZATION MACROS FOR DIFFERENT FIELD TYPES
// #DEFINE INIT_UINT128_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = HEX_STRING_TO_UINT128(VAL); \
//         } \
//     }
//
// #DEFINE INIT_UINT64_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = NUM2ULL(VAL); \
//         } \
//     }
//
// #DEFINE INIT_UINT32_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = NUM2UINT(VAL); \
//         } \
//     }
//
// #DEFINE INIT_UINT16_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = NUM2UINT(VAL); \
//         } \
//     }
//
// #DEFINE INIT_UINT8_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = (UINT8_T)NUM2INT(VAL); \
//         } \
//     }
//
// #DEFINE INIT_POINTER_FIELD(HASH, FIELD_NAME, OBJ_FIELD) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             OBJ->OBJ_FIELD = (VOID*)VAL; \
//         } \
//     }
//
// #DEFINE INIT_DATA_BUFFER_FIELD(HASH) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN("DATA"))); \
//         IF (!NIL_P(VAL)) { \
//             CHECK_TYPE(VAL, T_STRING); \
//             OBJ->DATA_SIZE = RSTRING_LEN(VAL); \
//             OBJ->DATA = XMALLOC(OBJ->DATA_SIZE); \
//             MEMCPY(OBJ->DATA, RSTRING_PTR(VAL), OBJ->DATA_SIZE); \
//         } \
//     }
//
// #DEFINE INIT_BYTE_ARRAY_FIELD(HASH, FIELD_NAME, OBJ_FIELD, ARRAY_SIZE) \
//     { \
//         VALUE VAL = RB_HASH_LOOKUP(HASH, ID2SYM(RB_INTERN(#FIELD_NAME))); \
//         IF (!NIL_P(VAL)) { \
//             CHECK_TYPE(VAL, T_STRING); \
//             SIZE_T COPY_SIZE = RSTRING_LEN(VAL) < ARRAY_SIZE ? RSTRING_LEN(VAL) : ARRAY_SIZE; \
//             MEMSET(OBJ->OBJ_FIELD, 0, ARRAY_SIZE); /* CLEAR EXISTING DATA */ \
//             MEMCPY(OBJ->OBJ_FIELD, RSTRING_PTR(VAL), COPY_SIZE); \
//         } \
//     }
//
// // COMPLETE CLASS DEFINITION MACRO
// #DEFINE DEFINE_TB_CLASS(KLASS_VAR, TYPE_NAME, RUBY_NAME, INITIALIZER_MACRO) \
//     KLASS_VAR = RB_DEFINE_CLASS_UNDER(RB_MTIGERBEETLE, RUBY_NAME, RB_COBJECT); \
//     DEFINE_STRUCT_ALLOCATOR(KLASS_VAR, TYPE_NAME); \
//     INITIALIZER_MACRO \
//     RB_DEFINE_METHOD(KLASS_VAR, "INITIALIZE", RB_##TYPE_NAME##_INITIALIZE, -1);
//
// // SUPER MACRO THAT GENERATES A COMPLETE CLASS WITH INITIALIZER AND ALL ACCESSORS
// #DEFINE DEFINE_COMPLETE_TB_CLASS(KLASS_VAR, TYPE_NAME, RUBY_NAME, ...) \
//     KLASS_VAR = RB_DEFINE_CLASS_UNDER(RB_MTIGERBEETLE, RUBY_NAME, RB_COBJECT); \
//     DEFINE_STRUCT_ALLOCATOR(KLASS_VAR, TYPE_NAME); \
//     DEFINE_STRUCT_INITIALIZER(TYPE_NAME, __VA_ARGS__) \
//     RB_DEFINE_METHOD(KLASS_VAR, "INITIALIZE", RB_##TYPE_NAME##_INITIALIZE, -1);
//
// // SPECIAL FREE FUNCTION FOR TB_PACKET TO HANDLE DATA FIELD
// STATIC VOID TB_PACKET_FREE(VOID *PTR) {
//     IF (PTR) {
//         TB_PACKET_T *PACKET = (TB_PACKET_T*)PTR;
//
//         // FREE DYNAMICALLY ALLOCATED DATA IF PRESENT
//         IF (PACKET->DATA != NULL) {
//             XFREE(PACKET->DATA);
//             PACKET->DATA = NULL;
//         }
//
//         // FREE THE PACKET STRUCT ITSELF
//         XFREE(PACKET);
//     }
// }
//
// // MEMORY SIZING FUNCTION FOR TB_PACKET
// STATIC SIZE_T TB_PACKET_MEMSIZE(CONST VOID *PTR) {
//     CONST TB_PACKET_T *PACKET = (CONST TB_PACKET_T*)PTR;
//     SIZE_T SIZE = SIZEOF(TB_PACKET_T);
//
//     // ADD SIZE OF DYNAMICALLY ALLOCATED DATA IF PRESENT
//     IF (PACKET->DATA != NULL) {
//         SIZE += PACKET->DATA_SIZE;
//     }
//
//     RETURN SIZE;
// }



// Helper function implementations
VALUE uint128_to_hex_string(tb_uint128_t value) {
    char buffer[33]; // 32 hex digits + null terminator
    uint64_t high = (uint64_t)(value >> 64);
    uint64_t low = (uint64_t)value;

    snprintf(buffer, sizeof(buffer), "%016" PRIx64 "%016" PRIx64, high, low);
    return rb_str_new_cstr(buffer);
}

tb_uint128_t hex_string_to_uint128(VALUE str) {
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

