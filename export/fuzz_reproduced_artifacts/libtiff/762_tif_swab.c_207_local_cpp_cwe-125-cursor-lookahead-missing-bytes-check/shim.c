// Auto-generated shim for 762_tif_swab.c_207_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabDouble();

int entry_func(double *dp) {
    TIFFSwabDouble(dp);
    return 0;
}
