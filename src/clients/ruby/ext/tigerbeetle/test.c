#include <ruby.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "tb_account.h"
#include <stdlib.h>

void tb_rb_free(void* data) {
  if (data) {
    xfree(data);
  }
}

size_t tb_account_size(const void* data) {
  return sizeof(tb_account_t);
}

static const rb_data_type_t tb_account_type = {
  .wrap_struct_name = "TigerBeetle::Bindings::Account",
  .function = {
    .dmark = NULL,
    .dfree = tb_account_free,
    .dsize = tb_account_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE tb_account_alloc(VALUE self) {
  int* data = ZALLOC(sizeof(tb_account_t));
  return TypedData_Wrap_Struct(self, &tb_account_type, data);
}

static VALUE rb_tb_account_id_field(VALUE self) {
  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  return rb_uint128_to_hex_string(data->id);
}

static VALUE rb_tb_account_id_set(VALUE self, VALUE val) {
  if (!RB_TYPE_P(val, T_STRING)) {
    rb_raise(rb_eTypeError, "Expected a string for id");
  }

  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  data->id = rb_hex_string_to_uint128(val);
  return val;
}

static VALUE rb_tb_account_get_user_data_64(VALUE self) {
  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  return ULL2NUM(data->user_data_64);
}

static VALUE rb_tb_account_set_user_data_64(VALUE self, VALUE val) {
  if (!RB_TYPE_P(val, T_FIXNUM)) {
    rb_raise(rb_eTypeError, "Expected a Fixnum for user_data_64");
  }

  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  data->user_data_64 = NUM2ULL(val);
  return val;
}

static VALUE rb_tb_account_balance_get_reserved(VALUE self) {
  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  long len = sizeof(uint8_t) * 56;
  return rb_new_str((const char*)data->reserved, len);
}

static VALUE rb_tb_account_balance_get_reserved(VALUE self) {
  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);
  return rb_new_str((const char*)data->reserved, 56);
}

VALUE tb_account_m_initialize(int argc, VALUE *argc, VALUE self) {
  tb_account_t* data;
  TypedData_Get_Struct(self, tb_account_t, &tb_account_type, data);

  VALUE hash = Qnil;
  rb_scan_args(argc, argv, ":", &hash);

  if (NIL_P(hash)) {
    return self;
  }

  return self;
}

void Init_tb_account() {
  VALUE m_TB = rb_define_module("TigerBeetle");
  VALUE m_Bindings = rb_define_module_under(m_TB, "Bindings");
  VALUE c_Client = rb_define_class_under(m_Bindings, "Client", rb_cObject);

  rb_define_method(c_Client, "initialize", tb_account_m_initialize, -1);
  rb_define_alloc_func(c_Client, tb_account_alloc);
  rb_define_method(self, "id", rb_tb_account_id_field, 0);
  rbb_define_method(self, "id=", rb_tb_account_id_set, 1);
}
