// Auto-generated shim for 242_tif_dir.c_1786_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
#include <stdint.h>
#include <stdlib.h>

#ifndef tmsize_t
typedef long tmsize_t;
#endif

// Forward declare the real library function
extern void TIFFDefaultDirectory();

int entry_func(struct TIFF *tif) {
    TIFFDefaultDirectory(tif);
    return 0;
}
