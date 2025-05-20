#include <ruby.h>
#include "tb_client.h"
#include "tb_macros.h"

// Implementation of helper functions declared in the header
VALUE uint128_to_hex_string(tb_uint128_t value) {
  char buffer[33]; // 32 hex digits + null terminator
  uint64_t high = (uint64_t)(value >> 64);
  uint64_t low = (uint64_t)value;

  snprintf(buffer, sizeof(buffer), "%016" PRIx64 "%016" PRIx64, high, low);
  return rb_str_new_cstr(buffer);
}

tb_uint128_t hex_string_to_uint128(VALUE str) {
  Check_Type(str, T_STRING);
  char *hex_str = StringValuePtr(str);
  tb_uint128_t result = 0;

  // Parse the hex string to 128-bit integer
  for (size_t i = 0; i < RSTRING_LEN(str); i++) {
    char c = hex_str[i];
    int val = 0;

    if (c >= '0' && c <= '9') val = c - '0';
    else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
    else continue; // Skip non-hex characters

    result = (result << 4) | val;
  }

  return result;
}
