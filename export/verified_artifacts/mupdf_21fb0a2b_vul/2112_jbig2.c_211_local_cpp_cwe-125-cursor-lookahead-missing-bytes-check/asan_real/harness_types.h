/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef byte
typedef unsigned char byte;
#endif

typedef struct Jbig2Ctx { int dummy; } Jbig2Ctx;

#define get_uint16(bptr) \
    (((bptr)[0] << 8) | (bptr)[1])

