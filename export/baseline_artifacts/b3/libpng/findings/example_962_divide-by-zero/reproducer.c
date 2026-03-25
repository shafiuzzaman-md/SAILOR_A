// Standalone C reproducer for divide-by-zero in example.c:write_png guard
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal libpng-like typedefs and macros to match the vulnerable code's types
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;
typedef struct png_struct_def* png_structp;

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFFu
#endif

// Constants used by the guard in the vulnerable code
#define PNG_SIZE_MAX ((png_uint_32)UINT32_MAX)
#define PNG_UINT_32_MAX ((png_uint_32)UINT32_MAX)

// Stub for png_error used by the code path (won't be reached due to div-by-zero)
static void png_error(png_structp png_ptr, const char *msg) {
    (void)png_ptr;
    fprintf(stderr, "png_error: %s\n", msg);
    exit(1);
}

// This function mirrors the vulnerable guard from example.c:write_png
// Specifically, it evaluates: if (height > PNG_SIZE_MAX / (width * bytes_per_pixel))
// Without guarding width*bytes_per_pixel != 0, passing width==0 triggers a divide-by-zero.
static void write_png(png_uint_32 width, png_uint_32 height, png_uint_32 bytes_per_pixel) {
    // The original code has additional setup calls; they are not necessary to trigger the bug.

    // Guard against integer overflow (vulnerable due to division by (width*bytes_per_pixel))
    if (height > PNG_SIZE_MAX / (width * bytes_per_pixel)) {
        png_error(NULL, "Image data buffer would be too large");
    }

    // The following allocations are present in the original code after the guard,
    // but are unreachable here because the divide-by-zero occurs when evaluating the guard.
    // Included only to reflect structure (not executed if crash occurs as intended):
    // png_byte image[height * width * bytes_per_pixel];
    // png_byte *row_pointers[height];
}

int main(void) {
    // Crafted input: set width=0 to make (width * bytes_per_pixel) == 0.
    // This forces a divide-by-zero in the guard expression before any error handling.
    png_uint_32 width = 0;            // Triggers denominator == 0
    png_uint_32 height = 10;          // Any non-zero value
    png_uint_32 bytes_per_pixel = 4;  // Typical RGBA bytes per pixel

    // Call the vulnerable function. On most systems, this will raise SIGFPE
    // due to integer divide-by-zero; with sanitizers, it will be reported accordingly.
    write_png(width, height, bytes_per_pixel);

    // Should not reach here
    puts("Unexpectedly survived divide-by-zero");
    return 0;
}
