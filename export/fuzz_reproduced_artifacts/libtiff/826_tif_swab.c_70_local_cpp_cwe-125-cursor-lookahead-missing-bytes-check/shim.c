// Auto-generated shim for 826_tif_swab.c_70_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabLong8();

int entry_func(uint64_t *lp) {
    extern void TIFFSwabLong8(uint64_t *lp);
    TIFFSwabLong8(lp);
    return 0;
}
