// Auto-generated shim for 791_tif_swab.c_173_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFSwabFloat();

int entry_func(float *fp) {
    TIFFSwabFloat(fp);  // DIRECT call, no guards
    return 0;
}
