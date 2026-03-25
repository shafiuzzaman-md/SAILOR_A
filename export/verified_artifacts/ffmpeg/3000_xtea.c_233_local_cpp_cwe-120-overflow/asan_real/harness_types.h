/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal type needed
typedef struct AVXTEA {
    uint32_t key[16];
} AVXTEA;

// Neutralized vulnerable function: keep signature and the vulnerable path with the exact memcpy line
static void xtea_crypt(AVXTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                       uint8_t *iv, int decrypt,
                       void (*crypt)(AVXTEA *, uint8_t *, const uint8_t *, int, uint8_t *))
{
    int i;
