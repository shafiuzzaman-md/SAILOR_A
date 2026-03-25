/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Minimal macro definition exactly as in source
#define BF(k) \
{\
    int a0, a1;\
    a0 = ptr[k];\
    a1 = ptr[8 + k];\
    ptr[k] = a0 + a1;\
    ptr[8 + k] = a0 - a1;\
}

// Vulnerable function (neutralized to only the vulnerable path)
