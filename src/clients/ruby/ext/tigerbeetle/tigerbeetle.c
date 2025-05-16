#include <ruby.h>
#include "tb_client.h"

tb_uint128_t rb_to_uint128(VALUE rb);

typedef struct {
  tb_client_t client;
  VALUE callback;
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

void rb_tb_completion_callback(uintptr_t ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* result_ptr, uint32_t result_len);
void handle_init_error_status(TB_INIT_STATUS status);
static VALUE rb_tb_client_initialize(int argc, VALUE *argv, VALUE self);

typedef struct {
  tb_account_t account;
  VALUE rb_data;
} rb_tb_account_wrapper;

static const rb_data_type_t rb_tb_account_type = {
}

/*
  * client = TigerBeetle::Client.new ...
  * flags = TigerBeetle::Bindings::ACCOUNT_FLAGS::ACCOUNT_DEBITS_MUST_NOT_EXCEED_CREDITS
  * data = [TigerBeetle::Bindings::Account.new(id: "123", user_data: "456", ledger: 10, code: 1, flags:)]
  * operation = TigerBeetle::Bindings::TB_OPERATION::CREATE_ACCOUNT
  * client.submit(data, operation)
  *
  * callback(ctx, packet, timestamp, result_ptr, result_len)
  *
  *
  */
static VALUE rb_tb_client_submit(VALUE self, VALUE rb_data, VALUE rb_operation);
static VALUE rb_tb_client_submit(VALUE self, VALUE rb_data, VALUE rb_operation) {
}

void Init_tigerbeetle() {
  VALUE mTigerBeetle = rb_define_module("TigerBeetle");
  VALUE rb_cClient = rb_define_class_under(mTigerBeetle, "Client", rb_cObject);
  rb_define_alloc_func(rb_cClient, rb_tb_client_alloc);
  rb_define_method(rb_cClient, "initialize", rb_tb_client_initialize, -1);
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
  rb_tb_client_wrapper *wrapper = (rb_tb_client_wrapper *)p;
  if (wrapper && !NIL_P(wrapper->callback)) {
    rb_gc_mark(wrapper->callback);
  }
}

static void rb_tb_client_free(void *p) {
  if (p) {
    rb_tb_client_wrapper *wrapper = (rb_tb_client_wrapper *)p;

    // NOTE: I think this should be handled elsewhere not on GC
    // if (wrapper->client) {
    //   tb_client_deinit(&wrapper->client);
    // }

    xfree(wrapper);
  }
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
  VALUE rb_addresses, rb_cluster_id, rb_callback, rb_echo;
  rb_scan_args(argc, argv, "31", &rb_addresses, &rb_cluster_id, &rb_callback, &rb_echo);

  Check_Type(rb_addresses, T_STRING);

  rb_tb_client_wrapper *wrapper;
  TypedData_Get_Struct(self, rb_tb_client_wrapper, &rb_tb_client_type, wrapper);

  if (!NIL_P(rb_callback) && !rb_respond_to(rb_callback, rb_intern("call"))) {
    rb_raise(rb_eTypeError, "callback must respond to 'call'");
  }
  wrapper->callback = rb_callback;

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
    (uintptr_t)self,
    rb_tb_completion_callback
  );
  handle_init_error_status(status);

  return self;
}

void rb_tb_completion_callback(uintptr_t ctx, tb_packet_t* packet, uint64_t timestamp, const uint8_t* result_ptr, uint32_t result_len) {
  rb_tb_client_wrapper *wrapper;
  TypedData_Get_Struct((VALUE)ctx, rb_tb_client_wrapper, &rb_tb_client_type, wrapper);

  if (NIL_P(wrapper->callback)) {
    rb_raise(rb_eRuntimeError, "No callback set");
  }

  /*
    * client.completion_callback(packet, timestamp, result_str)
    *
    */
  rb_funcall(wrapper->callback, rb_intern("call"), 1, (VALUE)ctx);
}
