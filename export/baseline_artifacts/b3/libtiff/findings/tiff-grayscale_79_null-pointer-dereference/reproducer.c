#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * This reproducer simulates the vulnerable portion of
 * contrib/dbs/tiff-grayscale.c:main around lines 76-81, where
 * malloc() is not checked and gray[0] is dereferenced unconditionally.
 * We force the allocation to fail deterministically and then
 * perform the same dereference to trigger a NULL pointer dereference.
 */

/* A malloc surrogate that always fails to emulate OOM */
static void *fail_malloc(size_t size) {
    (void)size;
    return NULL;
}

static void trigger_vuln_like_path(void) {
    int bits_per_pixel = 8;          /* as in the default case */
    int nchunks = 16;                /* for 8 bpp */
    int chunk_size = 32;             /* for 8 bpp */
    int cmsize;
    uint16_t *gray;

    (void)bits_per_pixel;            /* suppress unused warnings */
    (void)chunk_size;

    cmsize = nchunks * nchunks;      /* == 256 for 8 bpp */

    /* Force the vulnerable allocation to "fail" */
    /* In the original code: gray = (uint16_t*)malloc(cmsize * sizeof(uint16_t)); */
    gray = (uint16_t*)fail_malloc((size_t)cmsize * sizeof(uint16_t));

    /* Vulnerable dereference: mirrors `gray[0] = 3000;` at line 79 */
    gray[0] = 3000;  /* NULL pointer dereference */
}

int main(void) {
    /* Directly drive the vulnerable code path. */
    trigger_vuln_like_path();
    return 0;
}
