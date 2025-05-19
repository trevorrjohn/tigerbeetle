#include <ruby.h>
#include "tb_client.h"

const size_t MAX_BATCH_SIZE = 8189;

tb_uint128_t rb_to_uint128(VALUE rb);


typedef struct {
  tb_client_t client;
} rb_tb_client_wrapper;

static void rb_tb_client_mark(void *p);
static void rb_tb_client_free(void *p);
static size_t rb_tb_client_memsize(const void *p);
static VALUE rb_tb_client_alloc(VALUE klass);

static const rb_data_type_t rb_tb_client_type = {
  .wrap_struct_name = "TigerBeetle::Client",
  .function = {
    .dmark = rb_tb_client_mark,
    .dfree = rb_tb_client_free,
    .dsize = rb_tb_client_memsize,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

typedef struct {
  VALUE proc;
  VALUE self;
} rb_tb_proc_wrapper;

static void rb_tb_proc_mark(void *p);
static void rb_tb_proc_free(void *p);
static size_t rb_tb_proc_memsize(const void *p);
static const rb_data_type_t rb_tb_proc_type = {
  .wrap_struct_name = "TigerBeetle::Ext::ProcWrapper",
  .function = {
    .dmark = rb_tb_proc_mark,
    .dfree = rb_tb_proc_free,
    .dsize = rb_tb_proc_memsize,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

void rb_tb_completion_callback(uintptr_t _ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* result_ptr, uint32_t result_len);
void handle_init_error_status(TB_INIT_STATUS status);
static VALUE rb_tb_client_initialize(int argc, VALUE *argv, VALUE self);
static VALUE rb_tb_client_submit(int argc, VALUE *argv, VALUE self);

void Init_tigerbeetle() {
  VALUE tb_module = rb_define_module("TigerBeetle");
  VALUE bindings_module = rb_define_module_under(tb_module, "Bindings");
  VALUE rb_cClient = rb_define_class_under(bindings_module, "Client", rb_cObject);
  rb_define_alloc_func(rb_cClient, rb_tb_client_alloc);
  rb_define_method(rb_cClient, "initialize", rb_tb_client_initialize, -1);
  rb_define_method(rb_cClient, "submit", rb_tb_client_submit, -1);
}

tb_uint128_t rbignum_to_uint128(VALUE bignum);
tb_uint128_t rstring_to_uint128(VALUE rb_str);
tb_uint128_t rb_to_uint128(VALUE rb) {
  switch (TYPE(rb)) {
    case T_NIL:
      return 0;
      break;

    case T_FIXNUM:
      return (tb_uint128_t)NUM2ULL(rb);
      break;

    case T_BIGNUM:
      return rbignum_to_uint128(rb);
      break;

    case T_STRING:
      return rstring_to_uint128(rb);
      break;

    default:
      rb_unexpected_type(rb, -1);
      return 0;
  }
}

tb_uint128_t rbignum_to_uint128(VALUE bignum) {
  if (RBIGNUM_NEGATIVE_P(bignum)) { // will raise error if not bignum
    rb_raise(rb_eRangeError, "cannot convert negative number to unsigned 128-bit integer");
    return 0;
  }
  VALUE bit_length = rb_funcall(bignum, rb_intern("bit_length"), 0);
  size_t bits = NUM2SIZET(bit_length);

  if (bits > 128) {
    rb_raise(rb_eRangeError, "cannot convert a number larger than 128-bits");
    return 0;
  }

  if (bits <= 64) {
    return (tb_uint128_t)rb_big2ull(bignum);
  }

  VALUE lower_bits = rb_funcall(bignum, rb_intern("&"), 1, ULL2NUM(0xFFFFFFFFFFFFFFFF));
  uint64_t lower = rb_big2ull(lower_bits);

  VALUE upper_bits = rb_funcall(bignum, rb_intern(">>"), 1, INT2FIX(64));
  uint64_t upper = rb_big2ull(upper_bits);

  return ((tb_uint128_t)upper << 64) | lower;
}

tb_uint128_t rstring_to_uint128(VALUE rb_str) {
  Check_Type(rb_str, T_STRING);

  size_t len = RSTRING_LEN(rb_str);
  if (len == 0) {
    return 0;
  }

  char* str = StringValuePtr(rb_str);
  tb_uint128_t result = 0;
  int digits_count = 0;

  for (size_t i = 0; i < len; i++) {
    char c = str[i];
    if (c == '\0') break;

    // Skip underscores (Ruby allows 0xABCD_1234 format)
    // Skip hyphens (UUID's allowed)
    if (c == '_' || c == '-') continue;

    digits_count++;
    if (digits_count > 32) {
      rb_raise(rb_eRangeError, "String conversion to uint128 exceeds 128 bits");
      return 0;
    }

    // Perform the shift and addition
    result <<= 4;
    if (c >= '0' && c <= '9') {
      result |= (c - '0');
    } else if (c >= 'a' && c <= 'f') {
      result |= (c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      result |= (c - 'A' + 10);
    } else {
      rb_raise(rb_eArgError, "invalid hexadecimal character in string: '%c'", c);
    }
  }

  return result;
}

static void rb_tb_client_mark(void *p) {
  // noop
}

static void rb_tb_client_free(void *p) {
  if (p == NULL) return;

  rb_tb_client_wrapper *wrapper = (rb_tb_client_wrapper*)p;
  xfree(wrapper);
}

static size_t rb_tb_client_memsize(const void *p) {
  return sizeof(rb_tb_client_wrapper);
}

static VALUE rb_tb_client_alloc(VALUE klass) {
  rb_tb_client_wrapper *wrapper;
  wrapper = ZALLOC(rb_tb_client_wrapper);
  return TypedData_Wrap_Struct(klass, &rb_tb_client_type, wrapper);
}

void handle_init_error_status(TB_INIT_STATUS status) {
  switch(status) {
    case TB_INIT_SUCCESS:
      break;

    case TB_INIT_UNEXPECTED:
      rb_raise(rb_eRuntimeError, "Failed to initialize TigerBeetle client: Unexpected error");
      break;

    case TB_INIT_OUT_OF_MEMORY:
      rb_raise(rb_eNoMemError, "Failed to initialize TigerBeetle client: Out Of Memory error");
      break;

    case TB_INIT_ADDRESS_INVALID:
      rb_raise(rb_eArgError, "Failed to initialize TigerBeetle client: Address Invalid error");
      break;

    case TB_INIT_ADDRESS_LIMIT_EXCEEDED:
      rb_raise(rb_eArgError, "Failed to initialize TigerBeetle client: Limit Exceeded error");
      break;

    case TB_INIT_SYSTEM_RESOURCES:
      rb_raise(rb_eRuntimeError, "Failed to initialize TigerBeetle client: System Resources error");
      break;

    case TB_INIT_NETWORK_SUBSYSTEM:
      rb_raise(rb_eRuntimeError, "Failed to initialize TigerBeetle client: Network Subsystem error");
      break;

    default:
      rb_raise(rb_eRuntimeError, "Failed to initialize TigerBeetle client: Unknown error");
      break;
  }
}

static VALUE rb_tb_client_initialize(int argc, VALUE *argv, VALUE self) {
  VALUE rb_addresses, rb_cluster_id, rb_echo;
  rb_scan_args(argc, argv, "21", &rb_addresses, &rb_cluster_id, &rb_echo);

  Check_Type(rb_addresses, T_STRING);

  rb_tb_client_wrapper *wrapper;
  TypedData_Get_Struct(self, rb_tb_client_wrapper, &rb_tb_client_type, wrapper);

  char *addresses = StringValueCStr(rb_addresses);
  uint128_t cluster_id_value = rb_to_uint128(rb_cluster_id);
  const uint8_t *cluster_id = (const uint8_t *)&cluster_id_value;
  uint32_t address_len = (uint32_t)RSTRING_LEN(rb_addresses);

  TB_INIT_STATUS (*init_func)(
    tb_client_t*,
    const uint8_t[16],
    const char*,
    uint32_t,
    uintptr_t,
    void (*)(uintptr_t, tb_packet_t*, uint64_t, const uint8_t*, uint32_t)
  );
  if (RTEST(rb_echo)) {
    init_func = tb_client_init_echo;
  } else {
    init_func = tb_client_init;
  }

  TB_INIT_STATUS status = init_func(
    &wrapper->client,
    cluster_id,
    addresses,
    address_len,
    0, //completion ctx
    rb_tb_completion_callback
  );
  handle_init_error_status(status);

  return self;
}

void rb_tb_completion_callback(uintptr_t _ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* result_ptr, uint32_t result_len) {
  printf("rb_tb_completion_callback triggered");
  if (packet == NULL) {
    rb_raise(rb_eRuntimeError, "Packet is NULL");
    return;
  }

  rb_tb_proc_wrapper* wrapper = (rb_tb_proc_wrapper*)packet->user_data;
  if (!wrapper || !wrapper->self) {
    xfree(packet);
    rb_raise(rb_eRuntimeError, "Packet user_data freed before callback");
    return;
  }

  if (!rb_respond_to(wrapper->proc, rb_intern("call"))) {
    rb_gc_unregister_address(&wrapper->self);
    xfree(packet);
    rb_raise(rb_eTypeError, "callback must respond to 'call'");
    return;
  }

  // extract the result data
  VALUE status_code = INT2NUM(packet->status);
  VALUE result_at = ULL2NUM(timestamp);
  VALUE data;
  if (result_len > 0) {
    data = rb_str_new((const char *)result_ptr, result_len);
  } else {
    data = Qnil;
  }

  // create the result object
  VALUE mTB = rb_const_get(rb_cObject, rb_intern("TigerBeetle"));
  VALUE rbResult = rb_const_get(mTB, rb_intern("Result"));
  VALUE result = rb_funcall(rbResult, rb_intern("new"), 3, status_code, result_at, data);

  // call the passed to submit callback
  rb_funcall(wrapper->proc, rb_intern("call"), 1, result);

  // release wrapper and free packet
  rb_gc_unregister_address(&wrapper->self);
  xfree(packet->data);
  xfree(packet);
}

static size_t size_of_operation(TB_OPERATION op) {
  switch(op) {
    case TB_OPERATION_CREATE_ACCOUNTS:
      return sizeof(tb_account_t);
      break;
    case TB_OPERATION_CREATE_TRANSFERS:
      return sizeof(tb_transfer_t);
      break;
    default:
      rb_raise(rb_eArgError, "Invalid operation");
      return 0;
  }
}

void fill_field(VALUE field_name, VALUE field_type, VALUE member, void* dest, size_t* offset) {
  Check_Type(field_type, T_SYMBOL);

  VALUE field_type_str = rb_funcall(field_type, rb_intern("to_s"), 0);
  const char* type = StringValueCStr(field_type_str);

  if (strcmp(type, "uint8") == 0) {
    uint8_t val = NUM2UINT(member) & 0xFF;
    memcpy((char*)dest + *offset, &val, sizeof(val));
    *offset += sizeof(val);
  } else if (strcmp(type, "uint16") == 0) {
    uint16_t val = NUM2UINT(member) & 0xFFFF;
    memcpy((char*)dest + *offset, &val, sizeof(val));
    *offset += sizeof(val);
  } else if (strcmp(type, "uint32") == 0) {
    uint32_t val = NUM2UINT(member) & 0xFFFFFFFF;
    memcpy((char*)dest + *offset, &val, sizeof(val));
    *offset += sizeof(val);
  } else if (strcmp(type, "uint64") == 0) {
    uint64_t val = NUM2ULL(member);
    memcpy((char*)dest + *offset, &val, sizeof(val));
    *offset += sizeof(val);
  } else if (strcmp(type, "uint128") == 0) {
    tb_uint128_t val = rb_to_uint128(member);
    memcpy((char*)dest + *offset, &val, sizeof(val));
    *offset += sizeof(val);
  } else {
    rb_raise(rb_eArgError, "Unsupported field type: %s", type);
  }
}

size_t size_of_rb_type(char* type) {
  if (strcmp(type, "uint8") == 0) {
    return sizeof(uint8_t);
  } else if (strcmp(type, "uint16") == 0) {
    return sizeof(uint16_t);
  } else if (strcmp(type, "uint32") == 0) {
    return sizeof(uint32_t);
  } else if (strcmp(type, "uint64") == 0) {
    return sizeof(uint64_t);
  } else if (strcmp(type, "uint128") == 0) {
    return sizeof(tb_uint128_t);
  } else if (strcmp(type, "pointer") == 0) {
    return sizeof(char*);
  } else {
    rb_raise(rb_eArgError, "Unsupported field type: %s", type);
    return 0;
  }
}

// For a structure like your example
void fill_struct(VALUE field_name, VALUE field_type, VALUE member, void* dest, size_t* offset) {
  VALUE mTigerBeetle = rb_const_get(rb_cObject, rb_intern("TigerBeetle"));
  VALUE cEnum = rb_const_get(mTigerBeetle, rb_intern("CEnum"));
  if (RB_TYPE_P(field_type, T_ARRAY)) {
    VALUE base_type = rb_ary_entry(field_type, 0);
    size_t base_type_size = size_of_type(StringValueCStr(base_type));
    long count = NUM2LONG(rb_ary_entry(field_type, 1));

    if (NIL_P(member)) {
      memset((char*)dest + *offset, 0, count * sizeof(uint8_t));
      *offset += count * sizeof(uint8_t);
    }
  }
}

static VALUE rb_tb_client_submit(int argc, VALUE *argv, VALUE self) {
  VALUE rb_data, rb_operation;
  VALUE rb_block = Qnil;
  rb_scan_args(argc, argv, "2&", &rb_operation, &rb_data, &rb_block);

  if (NIL_P(rb_block)) {
    rb_raise(rb_eArgError, "Must provide a block to submit");
  }

  // need to create a callback wrapper and register for GC
  rb_tb_proc_wrapper *callback_wrapper;
  VALUE wrapper_obj = TypedData_Make_Struct(rb_cObject, rb_tb_proc_wrapper,
                                            &rb_tb_proc_type, callback_wrapper);
  callback_wrapper->proc = rb_block;
  callback_wrapper->self = wrapper_obj;
  rb_gc_register_address(&callback_wrapper->self);

  // create the packet data to be sent
  tb_packet_t *packet = (tb_packet_t *)ZALLOC(tb_packet_t);
  TB_OPERATION operation = (TB_OPERATION)(uint8_t)NUM2UINT(rb_operation);
  packet->operation = operation;

  long i;
  Check_Type(rb_data, T_ARRAY);
  size_t data_item_size = size_of_operation(operation);
  long len = RARRAY_LEN(rb_data);
  if (len > (long)MAX_BATCH_SIZE) {
    rb_raise(rb_eArgError, "data array is too large");
  }

  size_t data_size = data_item_size * (size_t)len;
  void* data = malloc(data_size);

  if (!data) {
    rb_raise(rb_eNoMemError, "Failed to allocate memory for data %zu bytes", data_size);
  }

  size_t offset = 0;
  for (i = 0; i < len; i++) {
    VALUE item = RARRAY_AREF(rb_data, i);
    // TODO check if this is the correct type
    VALUE fields = rb_funcall(item, rb_intern("fields"), 0);
    VALUE members = rb_funcall(item, rb_intern("members"), 0);

    for (int j = 0; j < RARRAY_LEN(fields); j++) {
      VALUE field_name = rb_ary_entry(fields, j);
      VALUE field_type = rb_hash_aref(fields, field_name);
      VALUE value = rb_hash_aref(members, field_name);

      fill_struct(field_name, field_type, value, data, &offset);
    }
  }

  packet->user_data = (void*)callback_wrapper;
  packet->data = data;
  packet->data_size = (uint32_t)data_size;

  // send the packet to the client
  rb_tb_client_wrapper* client_wrapper;
  TypedData_Get_Struct(self, rb_tb_client_wrapper, &rb_tb_client_type, client_wrapper);
  TB_CLIENT_STATUS status = tb_client_submit(&client_wrapper->client, packet);

  if (status == TB_CLIENT_OK) return Qtrue;

  VALUE mTB = rb_const_get(rb_cObject, rb_intern("TigerBeetle"));
  VALUE rbResult = rb_const_get(mTB, rb_intern("Result"));
  VALUE result = rb_funcall(rbResult, rb_intern("new"), 3, UINT2NUM(status), Qnil, Qnil);

  // send the failed result back to the block
  rb_funcall(rb_block, rb_intern("call"), 1, result);
  // there was an issue and the packet wasn't sent, free any memory allocated
  rb_gc_unregister_address(&callback_wrapper->self);
  free(packet->data);
  xfree(packet);
  return Qfalse;;
}

static void rb_tb_proc_mark(void *p) {
  if (p == NULL) return;

  rb_tb_proc_wrapper *wrapper = (rb_tb_proc_wrapper*)p;
  if (wrapper->proc != Qnil) {
    rb_gc_mark(wrapper->proc);
  }
}

static void rb_tb_proc_free(void *p) {
  if (p == NULL) return;

  rb_tb_proc_wrapper *wrapper = (rb_tb_proc_wrapper *)p;
  xfree(wrapper);
}

static size_t rb_tb_proc_memsize(const void *p) {
  return sizeof(rb_tb_proc_wrapper);
}
