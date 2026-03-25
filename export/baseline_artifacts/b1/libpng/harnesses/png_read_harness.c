/*
 * B1 Human-written harness: PNG Read (decode) path
 * Targets: png_read_info, png_read_image, png_read_end
 * Modeled after OSS-Fuzz png_read_fuzzer
 */
#include <png.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <klee/klee.h>

#define MAX_BUF_SIZE 256

struct mem_state {
    const unsigned char *data;
    size_t size;
    size_t offset;
};

static void mem_read_fn(png_structp png_ptr, png_bytep out, size_t count) {
    struct mem_state *state = (struct mem_state *)png_get_io_ptr(png_ptr);
    if (state->offset + count > state->size) {
        png_error(png_ptr, "read past end");
        return;
    }
    memcpy(out, state->data + state->offset, count);
    state->offset += count;
}

int main(void) {
    unsigned char buf[MAX_BUF_SIZE];
    klee_make_symbolic(buf, sizeof(buf), "png_input");

    /* Require valid PNG signature */
    buf[0] = 0x89;
    buf[1] = 0x50; /* P */
    buf[2] = 0x4E; /* N */
    buf[3] = 0x47; /* G */
    buf[4] = 0x0D;
    buf[5] = 0x0A;
    buf[6] = 0x1A;
    buf[7] = 0x0A;

    struct mem_state state = { buf, sizeof(buf), 0 };

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return 0;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    png_set_read_fn(png_ptr, &state, mem_read_fn);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);

    if (width > 64 || height > 64 || width == 0 || height == 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (rowbytes > 1024) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));
    if (!row_pointers) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    for (png_uint_32 i = 0; i < height; i++) {
        row_pointers[i] = (png_bytep)malloc(rowbytes);
        if (!row_pointers[i]) {
            for (png_uint_32 j = 0; j < i; j++) free(row_pointers[j]);
            free(row_pointers);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            return 0;
        }
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);

    for (png_uint_32 i = 0; i < height; i++) free(row_pointers[i]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 0;
}
