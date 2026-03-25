// Auto-generated shim for 818_tif_swab.c_90_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabArrayOfShort();

int harness_entry(uint16_t *wp, tmsize_t n) {
    TIFFSwabArrayOfShort(wp, n);
    return 0;
}
