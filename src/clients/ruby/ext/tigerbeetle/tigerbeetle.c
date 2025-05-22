#include <ruby.h>
#include "tb_client.h"
#include "tb_helpers.h"
#include "tb_bindings.h"

const size_t MAX_BATCH_SIZE = 8189;

tb_uint128_t rb_to_uint128(VALUE rb);

void completion_callback(uintptr_t ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* data, uint32_t data_size);

VALUE rb_tb_client_init(int argc, VALUE *argv, VALUE self);
VALUE rb_tb_client_submit(int argc, VALUE *argv, VALUE self);

void Init_tigerbeetle(void) {
  VALUE tb_module = rb_define_module("TigerBeetle");
  VALUE bindings_module = rb_define_module_under(tb_module, "Bindings");

  tb_define_enums_and_bitmasks(bindings_module);
  tb_define_ruby_accessors(bindings_module);

  VALUE tb_client_class = rb_const_get(bindings_module, rb_intern("Client"));
  rb_define_method(tb_client_class, "init", rb_tb_client_init, -1);
  rb_define_method(tb_client_class, "submit", rb_tb_client_submit, -1);
}

VALUE rb_tb_client_init(int argc, VALUE *argv, VALUE self) {
  VALUE kwargs;
  rb_scan_args(argc, argv, ":", &kwargs);

  tb_client_t *client;
  TypedData_Get_Struct(self, tb_client_t, &rb_tb_client_type, client);

  VALUE addresses = rb_hash_aref(kwargs, ID2SYM(rb_intern("addresses")));
  if (NIL_P(addresses)) {
    rb_raise(rb_eArgError, "addresses must be provided");
  }
  Check_Type(addresses, T_STRING);
  char *caddresses = StringValueCStr(addresses);
  VALUE rb_cluster_id =  rb_hash_aref(kwargs, ID2SYM(rb_intern("cluster_id")));
  tb_uint128_t cluster128_id = hex_string_to_uint128(rb_cluster_id);

  TB_INIT_STATUS status = tb_client_init(
    client,
    (uint8_t*)&cluster128_id,
    caddresses,
    (uint32_t)RSTRING_LEN(addresses),
    0,
    completion_callback
  );
  switch(status) {
    case TB_INIT_SUCCESS:
      return Qtrue;
      break;
    case TB_INIT_UNEXPECTED:
      rb_raise(rb_eRuntimeError, "Unexpected error during initialization");
      break;
    case TB_INIT_OUT_OF_MEMORY:
      rb_raise(rb_eNoMemError, "Out of memory");
      break;
    case TB_INIT_ADDRESS_INVALID:
      rb_raise(rb_eArgError, "Invalid address");
      break;
    case TB_INIT_ADDRESS_LIMIT_EXCEEDED:
      rb_raise(rb_eArgError, "Address limit exceeded");
      break;
    case TB_INIT_SYSTEM_RESOURCES:
      rb_raise(rb_eRuntimeError, "System resources error");
      break;
    case TB_INIT_NETWORK_SUBSYSTEM:
      rb_raise(rb_eRuntimeError, "Network subsystem error");
      break;
    default:
      rb_raise(rb_eRuntimeError, "Unknown error during initialization");
      break;
  }
  return Qfalse;
}

void completion_callback(uintptr_t ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* data, uint32_t data_size) {
  printf("Got the packet");
}

VALUE rb_tb_client_submit(int argc, VALUE *argv, VALUE self) {
  VALUE kwargs;
  rb_scan_args(argc, argv, ":", &kwargs);

  tb_client_t *client;
  TypedData_Get_Struct(self, tb_client_t, &rb_tb_client_type, client);

  tb_packet_t *packet;
  VALUE rb_packet = rb_hash_aref(kwargs, ID2SYM(rb_intern("packet")));
  TypedData_Get_Struct(rb_packet, tb_packet_t, &rb_tb_packet_type, packet);
  if (packet == NULL) {
    rb_raise(rb_eArgError, "rb_data must not be nil");
  }
  TB_CLIENT_STATUS status = tb_client_submit(client, packet);

  switch(status) {
    case TB_CLIENT_OK:
      return Qtrue;
      break;
    case TB_CLIENT_INVALID:
      rb_raise(rb_eRuntimeError, "Unexpected error during initialization");
      break;
    default:
      rb_raise(rb_eRuntimeError, "Unknown error during initialization");
      break;
  }
  return Qfalse;
}
