#include <ruby.h>
#include "tb_client.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Accepts a hex string and converts it to a 128-bit unsigned integer.
// allows for spaces and dashes in the string.
static tb_uint128_t rb_str_to_uint128(VALUE str) {
  Check_Type(str, T_STRING);
  const char *hex = StringValueCStr(str);
  tb_uint128_t value = 0;
  while (*hex) {
    char c = *hex++;
    if (c == ' ' || c == '-') continue;

    value <<= 4;
    if (c >= '0' && c <= '9') value |= (c - '0');
    else if (c >= 'a' && c <= 'f') value |= (c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') value |= (c - 'A' + 10);
    else rb_raise(rb_eArgError, "Invalid hex character");
  }
  return value;
}

static VALUE uint128_to_decimal_str(tb_uint128_t value) {
  char buffer[40];
  char *p = &buffer[sizeof(buffer) - 1];
  *p = '\0';
  if (value == 0) {
    *(--p) = '0';
  } else {
    while (value > 0) {
      *(--p) = '0' + (value % 10);
      value /= 10;
    }
  }
  return rb_str_new_cstr(p);
}

static VALUE tb_hex_to_decimal(VALUE self, VALUE hex_str) {
  tb_uint128_t value = rb_str_to_uint128(hex_str);
  return uint128_to_decimal_str(value);
}
