#include <ruby.h>
#include "tb_client.h"
#include <stdio.h>
#include <stdint.h>

/**
 * Convert a 128-bit unsigned integer to a hex string
 * Data is stored in little-endian format
 *
 * @param uint The 128-bit unsigned integer to convert
 * @return Ruby String object containing the hex representation
 */
VALUE uint128_to_hex_string(tb_uint128_t uint) {
    char hex_str[33]; // 32 hex chars + null terminator

    // Extract bytes in little-endian order
    uint8_t bytes[16];
    for (int i = 0; i < 16; i++) {
        bytes[i] = (uint >> (i * 8)) & 0xFF;
    }

    // Convert bytes to hex string (little-endian, so reverse order for display)
    for (int i = 0; i < 16; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[15 - i]);
    }
    hex_str[32] = '\0';

    return rb_str_new2(hex_str);
}

/**
 * Convert a hex string to a 128-bit unsigned integer
 * Supports UUID format (with hyphens), plain hex, and various prefixes
 * Data is stored in little-endian format
 *
 * @param hex_string Ruby String object containing the hex representation
 * @return tb_uint128_t The parsed 128-bit unsigned integer
 */
tb_uint128_t hex_string_to_uint128(VALUE hex_string) {
    tb_uint128_t result = 0;

    // Check if input is a string
    if (TYPE(hex_string) != T_STRING) {
        rb_raise(rb_eTypeError, "Expected a string");
        return result;
    }

    char *str = RSTRING_PTR(hex_string);
    long len = RSTRING_LEN(hex_string);

    // Create a working buffer to clean up the input
    char clean_hex[33] = {0}; // 32 hex chars + null terminator
    int clean_pos = 0;

    // Process the input string, removing hyphens, spaces, and prefixes
    for (int i = 0; i < len && clean_pos < 32; i++) {
        char c = str[i];

        // Skip common separators and prefixes
        if (c == '-' || c == ' ' || c == ':') {
            continue;
        }

        // Skip hex prefixes (0x, 0X)
        if (i == 0 && c == '0' && i + 1 < len &&
            (str[i + 1] == 'x' || str[i + 1] == 'X')) {
            i++; // Skip the 'x' in the next iteration
            continue;
        }

        // Convert to lowercase and validate hex character
        c = tolower(c);
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
            clean_hex[clean_pos++] = c;
        } else {
            rb_raise(rb_eArgError, "Invalid hex character: %c", str[i]);
            return result;
        }
    }

    // Pad with leading zeros if necessary (for shorter hex strings)
    if (clean_pos < 32) {
        memmove(clean_hex + (32 - clean_pos), clean_hex, clean_pos);
        memset(clean_hex, '0', 32 - clean_pos);
        clean_pos = 32;
    }

    // Validate length
    if (clean_pos > 32) {
        rb_raise(rb_eArgError, "Hex string too long (max 32 hex digits)");
        return result;
    }

    // Convert hex string to bytes (big-endian hex string to little-endian storage)
    uint8_t bytes[16];
    for (int i = 0; i < 16; i++) {
        char byte_str[3] = {clean_hex[i * 2], clean_hex[i * 2 + 1], '\0'};
        bytes[15 - i] = (uint8_t)strtoul(byte_str, NULL, 16);
    }

    // Pack bytes into __uint128_t in little-endian order
    for (int i = 0; i < 16; i++) {
        result |= ((tb_uint128_t)bytes[i]) << (i * 8);
    }

    return result;
}
