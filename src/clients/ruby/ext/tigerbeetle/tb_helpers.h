#ifndef TB_HELPERS_H
#define TB_HELPERS_H

#include <ruby.h>
#include "tb_client.h"
#include <stdio.h>
#include <stdint.h>

VALUE uint128_to_hex_string(tb_uint128_t uint);
tb_uint128_t hex_string_to_uint128(VALUE val);

#endif
