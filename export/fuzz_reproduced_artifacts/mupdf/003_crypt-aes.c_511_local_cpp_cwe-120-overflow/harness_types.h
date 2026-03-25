/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef FZ_AES_DECRYPT
#define FZ_AES_DECRYPT 0
#endif

// Minimal stand-in for the AES context (unused in our slice)
typedef struct aes_context { int nr; } aes_context;

// NEUTRALIZED vulnerable function: keep only the decrypt path and the sink line
void fz_aes_crypt_cbc( aes_context *ctx,
    int mode,
    size_t length,
    uint8_t iv[16],
    const uint8_t *input,
    uint8_t *output )
{
    (void)ctx; // unused in slice
    int i; (void)i; // keep decl to match original locals
    uint8_t temp[16];

