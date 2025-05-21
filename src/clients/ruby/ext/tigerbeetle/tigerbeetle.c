#include <ruby.h>
#include "tb_client.h"
#include "tb_bindings.h"

const size_t MAX_BATCH_SIZE = 8189;

tb_uint128_t rb_to_uint128(VALUE rb);

void Init_tigerbeetle(void) {
  VALUE tb_module = rb_define_module("TigerBeetle");
  VALUE bindings_module = rb_define_module_under(tb_module, "Bindings");

  tb_define_enums_and_bitmasks(bindings_module);
  tb_define_ruby_accessors(bindings_module);
}
