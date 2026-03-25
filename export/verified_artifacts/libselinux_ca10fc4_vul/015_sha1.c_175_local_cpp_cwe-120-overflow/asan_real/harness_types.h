/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal SHA1 context sufficient for Sha1Update
typedef struct {
    uint32_t State[5];
    uint32_t Count[2];
    uint8_t  Buffer[64];
} SHA1Context;

// External transform function (stubbed in stubs.c)

// Vulnerable function (neutralized, keep only core path and exact vulnerable line)
