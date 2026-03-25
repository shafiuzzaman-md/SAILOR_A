/*
 * B1 Human-written harness: PNG chunk handling
 * Targets: png_set_keep_unknown_chunks, chunk list manipulation,
 *          unknown chunk handling
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
    unsigned char chunk_list[20]; /* 4 chunk names of 5 bytes each */
    unsigned char keep_mode;
    unsigned char num_chunks;

    klee_make_symbolic(buf, sizeof(buf), "png_input");
    klee_make_symbolic(chunk_list, sizeof(chunk_list), "chunk_list");
    klee_make_symbolic(&keep_mode, sizeof(keep_mode), "keep_mode");
    klee_make_symbolic(&num_chunks, sizeof(num_chunks), "num_chunks");

    /* Valid PNG signature */
    buf[0] = 0x89; buf[1] = 0x50; buf[2] = 0x4E; buf[3] = 0x47;
    buf[4] = 0x0D; buf[5] = 0x0A; buf[6] = 0x1A; buf[7] = 0x0A;

    /* Constrain parameters */
    int keep = keep_mode % 4; /* PNG_HANDLE_CHUNK_AS_DEFAULT to _LAST */
    int n_chunks = (num_chunks % 4) + 1; /* 1 to 4 */

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

    /* Set up chunk keep list with symbolic chunk names */
    png_set_keep_unknown_chunks(png_ptr, keep,
                                chunk_list, n_chunks);

    png_set_read_fn(png_ptr, &state, mem_read_fn);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    if (width > 32 || height > 32 || width == 0 || height == 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (rowbytes > 512) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    png_bytep row = (png_bytep)malloc(rowbytes);
    if (!row) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    for (png_uint_32 y = 0; y < height; y++) {
        png_read_row(png_ptr, row, NULL);
    }

    free(row);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 0;
}
