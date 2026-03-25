#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Self-contained stubs and minimal types to reproduce the bug */

typedef long long toff_t;

typedef struct {
    toff_t tif_dataoff;
    uint32_t tif_flags;
} TIFF;

/* Local, internal-linkage versions to avoid symbol clashes with libtiff */
static void* _TIFFmalloc(size_t s) {
    /* Simulate allocation failure to trigger the NULL dereference path */
    (void)s;
    return NULL;
}

static void _TIFFmemcpy(void* dst, const void* src, size_t n) {
    /* Wrap memcpy to keep the call pattern identical to the vulnerable code */
    memcpy(dst, src, n);
}

/* Minimal reproduction of the vulnerable function logic around lines 218-221 */
static int TIFFWritePrivateDataSubDirectory(TIFF* tif,
                                            const uint32_t* pdir_fieldsset,
                                            uint32_t pdir_fields_last) {
    (void)tif; /* Unused in this minimal reproducer */

    /* fields_size is the number of uint32_t needed for the bit-mask */
    size_t fields_size = pdir_fields_last / (8 * sizeof(uint32_t)) + 1;

    /* Vulnerable allocation: result is not checked for NULL */
    uint32_t* fields = (uint32_t*)_TIFFmalloc(fields_size * sizeof(uint32_t));

    /* NULL-pointer-dereference: fields may be NULL on allocation failure */
    _TIFFmemcpy(fields, pdir_fieldsset, fields_size * sizeof(uint32_t));

    /* Not reached if the above memcpy dereferences NULL */
    return 1;
}

int main(void) {
    TIFF tif;
    tif.tif_dataoff = 0;
    tif.tif_flags = 0;

    /* Provide a tiny, valid pdir_fieldsset so the only fault is NULL dest */
    uint32_t pdir_fieldsset[1] = {0};

    /* Choose pdir_fields_last small so copy length is small (4 bytes),
       but make _TIFFmalloc always fail so we deterministically deref NULL */
    uint32_t pdir_fields_last = 0; /* fields_size = 1; copy = 4 bytes */

    /* This call will trigger the vulnerable memcpy with a NULL destination */
    (void)TIFFWritePrivateDataSubDirectory(&tif, pdir_fieldsset, pdir_fields_last);

    /* If we somehow return, indicate unexpected behavior */
    puts("Unexpectedly returned without crashing");
    return 0;
}
