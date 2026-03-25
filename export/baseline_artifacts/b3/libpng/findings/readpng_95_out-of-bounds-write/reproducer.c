#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal libpng-like typedefs and API stubs to mimic the real code path */
typedef unsigned char png_byte;      /* byte */
typedef unsigned char* png_bytep;    /* byte pointer */
typedef uint32_t png_uint_32;

typedef struct png_struct_def { int dummy; } *png_structp;
typedef struct png_info_def { int dummy; } *png_infop;

enum { PNG_HANDLE_CHUNK_ALWAYS = 3 };

/* Public API stubs */
png_structp png_create_read_struct(const char *ver, void *error_ptr,
                                   void *error_fn, void *warn_fn) {
    (void)ver; (void)error_ptr; (void)error_fn; (void)warn_fn;
    png_structp p = (png_structp)malloc(sizeof(*p));
    return p;
}

png_infop png_create_info_struct(png_structp png_ptr) {
    (void)png_ptr;
    png_infop p = (png_infop)malloc(sizeof(*p));
    return p;
}

void png_destroy_read_struct(png_structp *png_ptr, png_infop *info_ptr,
                             png_infop *end_info) {
    if (png_ptr && *png_ptr) { free(*png_ptr); *png_ptr = NULL; }
    if (info_ptr && *info_ptr) { free(*info_ptr); *info_ptr = NULL; }
    (void)end_info;
}

void png_error(png_structp png_ptr, const char *msg) {
    (void)png_ptr;
    fprintf(stderr, "png_error: %s\n", msg);
    /* In real libpng this longjmps; for our stub, just abort. */
    abort();
}

void png_set_keep_unknown_chunks(png_structp png_ptr, int keep, void *list, int num) {
    (void)png_ptr; (void)keep; (void)list; (void)num;
}

void png_read_info(png_structp png_ptr, png_infop info_ptr) {
    (void)png_ptr; (void)info_ptr;
}

size_t png_get_rowbytes(png_structp png_ptr, png_infop info_ptr) {
    (void)png_ptr; (void)info_ptr;
    /* Return a non-zero row size to trigger allocations */
    return 1024;
}

png_uint_32 png_get_image_height(png_structp png_ptr, png_infop info_ptr) {
    (void)png_ptr; (void)info_ptr;
    return 1; /* single row is sufficient to trigger the bug */
}

int png_set_interlace_handling(png_structp png_ptr) {
    (void)png_ptr;
    return 1; /* single pass */
}

void png_start_read_image(png_structp png_ptr) { (void)png_ptr; }

/* The critical function that will be called with an invalid pointer for 'display' */
void png_read_row(png_structp png_ptr, png_bytep row, png_bytep display) {
    (void)png_ptr;
    /* Simulate libpng writing a row and an optional display row */
    if (row) {
        row[0] = 0xAA;
    }
    if (display) {
        /* This write will target an invalid pointer due to truncation. */
        display[0] = 0xBB;
    }
}

void png_read_end(png_structp png_ptr, png_infop info_ptr) { (void)png_ptr; (void)info_ptr; }

/* ------------------ Reproducer of the buggy function ------------------ */

/* This mimics contrib/libtests/readpng.c:read_png with the specific bug:
 * 'display' is a png_byte (byte) instead of a png_bytep (pointer).
 */
int read_png(FILE *file) {
    (void)file; /* Unused in this stub-based reproducer */

    png_structp png_ptr = png_create_read_struct("1.6.0", NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_bytep row = NULL;
    png_byte display; /* BUG: should be png_bytep */

    png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);
    png_read_info(png_ptr, info_ptr);

    {
        size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

        /* Allocate buffers */
        row = (png_bytep)malloc(rowbytes);
        /* BUG: pointer value truncated to a single byte */
        display = (png_byte)(uintptr_t)malloc(rowbytes);

        /* Only check row for NULL to ensure we don't exit early by chance
         * if the low byte of the pointer happens to be 0. */
        if (row == NULL) png_error(png_ptr, "OOM allocating row buffer");

        {
            png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
            int passes = png_set_interlace_handling(png_ptr);
            int pass;

            png_start_read_image(png_ptr);

            for (pass = 0; pass < passes; ++pass) {
                png_uint_32 y = height;
                while (y-- > 0) {
                    /* Pass the truncated value back as a pointer: invalid! */
                    png_read_row(png_ptr, row, (png_bytep)(uintptr_t)display);
                }
            }
        }
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* Free the valid row; also attempt to free the invalid pointer, which would
     * be another bug, but we likely crash earlier during png_read_row. */
    free(row);
    free((void*)(uintptr_t)display);

    return 1;
}

int main(void) {
    /* Exit code 0 on success. */
    return !read_png(stdin);
}
