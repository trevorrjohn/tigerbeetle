#include <ruby.h>
#include <ruby/thread.h>
#include <ruby/io.h>
#include "tb_client.h"
#include "tb_helpers.h"
#include "tb_bindings.h"

const size_t MAX_BATCH_SIZE = 8189;

typedef struct {
  VALUE queue;
} rb_callback_t;

void completion_callback(
    uintptr_t context,
    tb_packet_t *packet,
    uint64_t timestamp,
    const uint8_t *data,
    uint32_t size
);

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

void log_callback(TB_LOG_LEVEL level, const uint8_t* message, uint32_t size) {
  // const char* level_str;
  // switch(level) {
  //   case TB_LOG_DEBUG:   level_str = "DEBUG"; break;
  //   case TB_LOG_INFO:    level_str = "INFO";  break;
  //   case TB_LOG_WARN:    level_str = "WARN";  break;
  //   case TB_LOG_ERR:     level_str = "ERROR"; break;
  //   default:             level_str = "UNKNOWN"; break;
  // }

  // printf("[%s] %.*s\n", level_str, (int)size, (const char*)message);
}

typedef struct {
  TB_INIT_STATUS status;
  tb_client_t* client;
  uint8_t* cluster_id;
  char* addresses;
  uint32_t addresses_len;
} init_parameters;

void* rb_tb_client_init_without_gvl(void* data) {
  init_parameters* args = (init_parameters*)data;

  printf("Connceting to client\n");
  args->status = tb_client_init(
    args->client,
    args->cluster_id,
    args->addresses,
    args->addresses_len,
    (uintptr_t)NULL,
    &completion_callback
  );

  return NULL;
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

  init_parameters args;
  args.client = client;
  args.cluster_id = (uint8_t*)&cluster128_id;
  args.addresses = caddresses;
  args.addresses_len = (uint32_t)RSTRING_LEN(addresses);
  (void)rb_thread_call_without_gvl(
    rb_tb_client_init_without_gvl,
    &args,
    NULL,
    NULL
  );

  switch(args.status) {
    case TB_INIT_SUCCESS:
      tb_client_register_log_callback(&log_callback, true);
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

typedef struct {
  int status;
  TB_OPERATION operation;
  rb_callback_t* callback;
  char* data;
  uint32_t size;
} completion_result;

static void* completion_callback_with_gvl(void* data) {
  printf("completion callback with gvl\n");
  completion_result* res = (completion_result*) data;
  VALUE rb_results = Qnil;
  switch(res->operation) {
    case TB_OPERATION_CREATE_ACCOUNTS:
      rb_raise(rb_eRuntimeError, "Not yet implemented");
      break;
    case TB_OPERATION_LOOKUP_ACCOUNTS:
      ;
      int results_len = res->size / sizeof(tb_account_t);
      tb_account_t *results = (tb_account_t*)res->data;
      rb_results = rb_ary_new2(results_len);
      printf("%d Account(s) found\n", results_len);
      printf("============================================\n");

      for(int i=0;i<results_len;i++) {
        rb_ary_push(rb_results, uint128_to_hex_string(results[i].id));
        printf("id=%ld\n", (long)results[i].id);
        printf("debits_posted=%ld\n", (long)results[i].debits_posted);
        printf("credits_posted=%ld\n", (long)results[i].credits_posted);
        printf("\n");
      }
      res->status = 0;
      break;
    default:
      res->status = 1;
      break;
  }

  printf("here here here\n");
  rb_funcall(res->callback->queue, rb_intern("push"), 1, rb_results);
  return NULL;
}

void completion_callback(
  uintptr_t context,
  tb_packet_t *packet,
  uint64_t timestamp,
  const uint8_t *data,
  uint32_t size
) {
  printf("Got the packet\n");
  completion_result res;
  res.operation = (TB_OPERATION)packet->operation;
  res.callback = (rb_callback_t*)packet->user_data;
  res.data = (char*)malloc(size);
  if (res.data == NULL) {
    printf("Out of memory\n");
    return;
  }
  memcpy(&res.data, data, size);
  res.size = size;

  if (rb_thread_alone()) {
    printf("Thread is alone\n");
  } else {
    printf("Thread is not alone\n");
  }

  (void)rb_thread_call_with_gvl(completion_callback_with_gvl, &res);

  if (res.status == 0) {
    printf("good\n");
  } else {
    printf("bad\n");
  }
}

typedef struct {
  TB_CLIENT_STATUS status;
  tb_client_t* client;
  tb_packet_t* packet;
} submit_parameters;

void* rb_tb_client_submit_without_gvl(void* data) {
  submit_parameters* args = (submit_parameters*) data;
  tb_client_t* client = args->client;
  tb_packet_t* packet = args->packet;
  printf("submitting packet\n");
  args->status = tb_client_submit(client, packet);
  return NULL;
}

VALUE rb_tb_client_submit(int argc, VALUE *argv, VALUE self) {
  VALUE rb_operation, rb_data_array, rb_queue;
  rb_scan_args(argc, argv, "3", &rb_operation, &rb_data_array, &rb_queue);

  Check_Type(rb_operation, T_FIXNUM);
  Check_Type(rb_data_array, T_ARRAY);
  if (!NIL_P(rb_queue) && !rb_respond_to(rb_queue, rb_intern("push"))) {
    rb_raise(rb_eArgError, "Queue must respond to #push");
  }

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
      if (data == NULL) {
        rb_raise(rb_eNoMemError, "Out of memory");
        return Qnil;
      }
      memset(data, 0, data_size);
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
    case TB_OPERATION_LOOKUP_ACCOUNTS:
      data_size = (uint32_t)sizeof(tb_uint128_t) * (uint32_t)data_array_size;
      data = malloc(data_size);
      if (data == NULL) {
        rb_raise(rb_eNoMemError, "Out of memory");
        return Qnil;
      }
      memset(data, 0, data_size);
      tb_uint128_t account_id;
      for (int i = 0; i < (int)data_array_size; i++) {
        item = rb_ary_entry(rb_data_array, i);
        account_id = hex_string_to_uint128(item);
        memcpy(((tb_account_t*)data + (i * sizeof(tb_uint128_t))), &account_id, sizeof(tb_uint128_t));
      }
      break;
    default:
      rb_raise(rb_eRuntimeError, "Operation Not yet implemented: %d", operation);
      return Qnil;
      break;

  }

  rb_callback_t *callback = (rb_callback_t*)malloc(sizeof(rb_callback_t));
  if (callback == NULL) {
    rb_raise(rb_eNoMemError, "Out of memory");
  }
  callback->queue = rb_queue;

  tb_packet_t *packet = (tb_packet_t*)malloc(sizeof(tb_packet_t));
  if (packet == NULL) {
    rb_raise(rb_eNoMemError, "Out of memory");
  }

  packet->user_data = (void*)callback;
  packet->data = data;
  packet->data_size = data_size;
  packet->operation = operation;
  packet->status = TB_PACKET_OK;

  submit_parameters args;
  args.client = client;
  args.packet = packet;
  (void)rb_thread_call_without_gvl(
    rb_tb_client_submit_without_gvl,
    &args,
    NULL,
    NULL
  );

  switch(args.status) {
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
  free(callback);
  return Qfalse;
}
