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
  VALUE rb_operation, rb_data_array;
  VALUE rb_block = Qnil;
  rb_scan_args(argc, argv, "2&", &rb_operation, &rb_data_array, &rb_block);

  Check_Type(rb_operation, T_FIXNUM);
  Check_Type(rb_data_array, T_ARRAY);
  rb_need_block();

  tb_client_t *client;
  TypedData_Get_Struct(self, tb_client_t, &rb_tb_client_type, client);

  if (client == NULL)  {
    rb_raise(rb_eRuntimeError, "tb_client_t was not initialized");
    return Qnil;
  }

  uint8_t operation = (uint8_t)NUM2UINT(rb_operation);
  void* data;
  uint32_t data_size = 0;
  long data_array_size = RARRAY_LEN(rb_data_array);
  if (data_array_size > (long)MAX_BATCH_SIZE) {
    rb_raise(rb_eArgError, "Data batch exceeds max batch size %ld", data_array_size);
    return Qnil;
  }

  VALUE item;
  switch((TB_OPERATION)operation) {
    case TB_OPERATION_CREATE_ACCOUNTS:
      data_size = (uint32_t)sizeof(tb_account_t) * (uint32_t)data_array_size;
      data = malloc(data_size);
      tb_account_t* account_t;
      for (int i = 0; i < (int)data_array_size; i++) {
        item = rb_ary_entry(rb_data_array, i);

        TypedData_Get_Struct(item, tb_account_t, &rb_tb_account_type, account_t);
        if (account_t == NULL) {
          rb_raise(rb_eTypeError, "Invalid data at %d, expecting Account", (int)i);
          return Qnil;
        }
        memcpy(((tb_account_t*)data + (i * sizeof(tb_account_t))), account_t, sizeof(tb_account_t));
      }
    break;
    default:
      rb_raise(rb_eRuntimeError, "Operation Not yet implemented: %d", operation);
      break;

  }

  tb_packet_t *packet = (tb_packet_t*)malloc(sizeof(tb_packet_t));
  if (packet == NULL) {
    rb_raise(rb_eNoMemError, "Out of memory");
  }
  packet->data = data;
  packet->data_size = data_size;
  packet->operation = operation;
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
