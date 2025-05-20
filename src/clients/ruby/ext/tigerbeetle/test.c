#include <ruby.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "tb_client.h"
#include <stdlib.h>

void tb_client_free(void* data)
{
	free(data);
}

size_t tb_client_size(const void* data)
{
	return sizeof(tb_client_t);
}

static const rb_data_type_t tb_client_type = {
	.wrap_struct_name = "TigerBeetle::Bindings::Client",
	.function = {
		.dmark = NULL,
		.dfree = tb_client_free,
		.dsize = tb_client_size,
	},
	.data = NULL,
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE tb_client_alloc(VALUE self)
{
	/* allocate */
	int* data = malloc(sizeof(tb_client_t));

	/* wrap */
	return TypedData_Wrap_Struct(self, &tb_client_type, data);
}

VALUE tb_client_m_initialize(int argc, VALUE *argc, VALUE self)
{
	tb_client_t* data;
	/* unwrap */
	TypedData_Get_Struct(self, tb_client_t, &tb_client_type, data);

  VALUE hash = Qnil;
  rb_scan_args(argc, argv, ":", &hash);

  if (NIL_P(hash)) {
    // no data provided so just return self
    return self;
  }

  static VALUE rb_tb_client_id_field(VALUE self) {
    tb_client_t* data;
    TypedData_Get_Struct(self, tb_client_t, &tb_client_type, data);
    return rb_uint128_to_hex_string(data->id);
  }
  static VALUE rb_tb_client_id_set(VALUE self, VALUE val) {
    if (!RB_TYPE_P(val, T_STRING)) {
      rb_raise(rb_eTypeError, "Expected a string for id");
    }

    tb_client_t* data;
    TypedData_Get_Struct(self, tb_client_t, &tb_client_type, data);

    if (NIL_P(val)) {
      data->id = NULL;
    } else {
      data->id = rb_hex_string_to_uint128(val);
    }

    return val;
  }

	return self;
}

