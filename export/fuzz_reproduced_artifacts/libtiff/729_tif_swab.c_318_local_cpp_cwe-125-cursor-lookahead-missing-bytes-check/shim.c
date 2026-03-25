// Auto-generated shim for 729_tif_swab.c_318_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFReverseBits();

int entry_func(uint8_t *cp, tmsize_t n) {
    TIFFReverseBits(cp, n);
    return 0;
}
