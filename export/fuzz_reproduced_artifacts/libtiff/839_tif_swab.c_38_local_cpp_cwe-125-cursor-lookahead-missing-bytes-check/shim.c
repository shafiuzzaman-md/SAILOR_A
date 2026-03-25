// Auto-generated shim for 839_tif_swab.c_38_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabShort();

int harness_entry(uint16_t *wp) {
    TIFFSwabShort(wp);
    return 0;
}
