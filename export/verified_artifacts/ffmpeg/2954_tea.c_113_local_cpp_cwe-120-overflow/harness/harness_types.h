/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal local type definition
typedef struct AVTEA {
    uint32_t key[16];
    int rounds;
} AVTEA;

// Neutralized helper: ensure dst gets 8 bytes written deterministically
static void tea_crypt_ecb(AVTEA *ctx, uint8_t *dst, const uint8_t *src,
