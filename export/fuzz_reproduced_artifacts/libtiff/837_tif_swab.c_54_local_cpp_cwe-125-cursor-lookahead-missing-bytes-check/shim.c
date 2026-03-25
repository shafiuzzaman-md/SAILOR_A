// Auto-generated shim for 837_tif_swab.c_54_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabLong();

int entry_func(uint32_t *lp) {
    extern void TIFFSwabLong(uint32_t *lp);
    TIFFSwabLong(lp);  // DIRECT call, no guards
    return 0;
}
