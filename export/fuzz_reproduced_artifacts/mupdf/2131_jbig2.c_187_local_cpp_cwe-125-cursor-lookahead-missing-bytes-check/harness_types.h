/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>

typedef unsigned char byte;

#define get_uint16(bptr) \
    (((bptr)[0] << 8) | (bptr)[1])
#define get_int16(bptr) \
    (((int)get_uint16(bptr) ^ 0x8000) - 0x8000)

int16_t
