// Auto-generated shim for 795_tif_swab.c_147_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabArrayOfLong8();

int entry_func(uint64_t *lp, tmsize_t n) {
    TIFFSwabArrayOfLong8(lp, n);
    return 0;
}
