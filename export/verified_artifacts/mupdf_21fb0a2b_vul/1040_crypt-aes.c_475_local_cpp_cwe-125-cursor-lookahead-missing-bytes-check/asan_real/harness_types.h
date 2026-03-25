/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal stand-in for project type; fields unused in harness
typedef struct { char opaque[128]; } aes_context;

// Local definition of macro used at the sink site
#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i) do { \
    (b)[(i)    ] = (uint8_t)(((n)      ) & 0xFF); \
    (b)[(i) + 1] = (uint8_t)(((n) >>  8) & 0xFF); \
    (b)[(i) + 2] = (uint8_t)(((n) >> 16) & 0xFF); \
    (b)[(i) + 3] = (uint8_t)(((n) >> 24) & 0xFF); \
} while (0)
#endif

// Vulnerable function (neutralized) — keep only the sink statement verbatim
