/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif
#ifndef AES_CTR_IV_SIZE
#define AES_CTR_IV_SIZE 16
#endif

// Minimal struct with only fields needed on path
struct AVAESCTR {
    uint8_t *counter;                       // pointer-typed to reflect potential bug scenario
    uint8_t encrypted_counter[AES_BLOCK_SIZE];
    int block_offset;
};

