#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

/* We simulate the libpng API surface minimally so that we can compile and
 * exercise the vulnerable code path in contrib/gregbook/readpng2.c:readpng2_info_callback.
 * The key bug is casting png_get_rowbytes() (size_t) to int, truncating/overflowing.
 */

#define PNG_FLOATING_POINT_SUPPORTED 1

typedef struct png_struct_def png_struct;
typedef struct png_info_def png_info;
typedef unsigned char png_byte;
typedef uint32_t png_uint_32;

/* Front-end struct used by the gregbook code; we only model necessary fields. */
typedef struct mainprog_info_s {
    int rowbytes;                 /* vulnerable field: assigned from size_t then used */
    int channels;
    int passes;
    double display_exponent;
    void (*mainprog_init)(void);  /* callback to front-end */
    unsigned char *image_row;     /* buffer allocated using truncated rowbytes */
} mainprog_info;

/* Global instance to be returned by png_get_progressive_ptr() stub */
static mainprog_info gMain;

/* Choose a very large rowbytes value whose low 32 bits are small/positive so that
 * (int)cast truncation yields a small allocation, but the true row size is huge.
 * Example: (1<<33) + 1024 -> low 32 bits = 1024, high bits non-zero.
 */
static const size_t BIG_ROWBYTES = (((size_t)1) << 33) + 1024; /* ~8 GiB + 1 KiB */

/* ---- Minimal stubs for libpng API used by readpng2_info_callback ---- */
int png_get_gAMA(png_struct *png_ptr, png_info *info_ptr, double *gamma) {
    (void)png_ptr; (void)info_ptr; (void)gamma;
    return 0; /* behave as if image is unlabelled; take sRGB default path */
}

void png_set_gamma(png_struct *png_ptr, double display_exponent, double gamma) {
    (void)png_ptr; (void)display_exponent; (void)gamma; /* no-op */
}

int png_set_interlace_handling(png_struct *png_ptr) {
    (void)png_ptr; return 1; /* one pass for simplicity */
}

void png_read_update_info(png_struct *png_ptr, png_info *info_ptr) {
    (void)png_ptr; (void)info_ptr; /* no-op */
}

size_t png_get_rowbytes(png_struct *png_ptr, png_info *info_ptr) {
    (void)png_ptr; (void)info_ptr; return BIG_ROWBYTES; /* the critical large value */
}

int png_get_channels(png_struct *png_ptr, png_info *info_ptr) {
    (void)png_ptr; (void)info_ptr; return 4; /* e.g., RGBA */
}

void *png_get_progressive_ptr(png_struct *png_ptr) {
    (void)png_ptr; return &gMain; /* return our front-end state */
}

/* ---- Vulnerable function re-implemented from contrib/gregbook/readpng2.c ---- */
static void readpng2_info_callback(png_struct *png_ptr, png_info *info_ptr)
{
    mainprog_info *mainprog_ptr = (mainprog_info *)png_get_progressive_ptr(png_ptr);
    double gamma;

#ifdef PNG_FLOATING_POINT_SUPPORTED
    if (png_get_gAMA(png_ptr, info_ptr, &gamma))
        png_set_gamma(png_ptr, mainprog_ptr->display_exponent, gamma);
    else
        png_set_gamma(png_ptr, mainprog_ptr->display_exponent, 0.45455);
#else
    /* Not used in this reproducer */
#endif

    mainprog_ptr->passes = png_set_interlace_handling(png_ptr);

    /* Update info then fetch rowbytes and channels. This is where the bug is:
     * png_get_rowbytes() returns size_t but it is cast to int and stored. */
    png_read_update_info(png_ptr, info_ptr);

    mainprog_ptr->rowbytes = (int)png_get_rowbytes(png_ptr, info_ptr); /* BUG: truncates */
    mainprog_ptr->channels = png_get_channels(png_ptr, info_ptr);

    /* Invoke front-end to allocate buffers based on truncated rowbytes. */
    (*mainprog_ptr->mainprog_init)();
}

/* ---- Front-end code that allocates using the truncated rowbytes ---- */
static void frontend_init(void)
{
    /* Allocate image row buffer using truncated int-sized rowbytes. */
    size_t alloc = (size_t)gMain.rowbytes;
    printf("[frontend_init] Truncated rowbytes (int) = %d, allocating %zu bytes\n",
           gMain.rowbytes, alloc);
    gMain.image_row = (unsigned char *)malloc(alloc);
    if (!gMain.image_row) {
        fprintf(stderr, "Allocation failed for %zu bytes\n", alloc);
        exit(1);
    }
    memset(gMain.image_row, 0xCC, alloc);
}

int main(void)
{
    /* Initialize front-end state and function pointer. */
    memset(&gMain, 0, sizeof(gMain));
    gMain.display_exponent = 2.2; /* typical display exponent */
    gMain.mainprog_init = &frontend_init;

    /* Call the vulnerable info callback, which will store rowbytes as int and
     * call frontend_init() to allocate based on the truncated value. */
    readpng2_info_callback((png_struct *)0x1, (png_info *)0x1);

    /* Show the true rowbytes as libpng would see them (size_t). */
    size_t true_rowbytes = png_get_rowbytes(NULL, NULL);
    printf("[main] True rowbytes (size_t) = %zu, truncated stored = %d\n",
           true_rowbytes, gMain.rowbytes);

    /* Simulate libpng writing a full-sized row into the under-allocated buffer.
     * We don't actually loop for 'true_rowbytes' bytes (which is huge). Instead,
     * we write a small chunk beyond the end to trigger ASan, demonstrating that
     * the allocation was too small for the real row size. */
    size_t truncated = (size_t)gMain.rowbytes;      /* e.g., 1024 */
    size_t to_overwrite = 4096;                     /* write 4 KiB total */

    printf("[main] Writing %zu bytes into buffer of %zu bytes to simulate overflow...\n",
           to_overwrite, truncated);

    /* This write goes past the end by (to_overwrite - truncated) bytes. */
    for (size_t i = 0; i < to_overwrite; ++i) {
        gMain.image_row[i] = (unsigned char)(i & 0xFF);
    }

    /* Clean up (won't be reached if ASan catches the overflow earlier). */
    free(gMain.image_row);
    return 0;
}
