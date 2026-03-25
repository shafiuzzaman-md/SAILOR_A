/*
 * B1 Human-written harness: PNG Write (encode) path
 * Targets: png_write_info, png_write_row, png_write_end
 */
#include <png.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <klee/klee.h>

#define MAX_DIM 16
#define MAX_ROWBYTES 256

struct write_state {
    unsigned char buf[4096];
    size_t offset;
};

static void mem_write_fn(png_structp png_ptr, png_bytep data, size_t length) {
    struct write_state *state = (struct write_state *)png_get_io_ptr(png_ptr);
    if (state->offset + length <= sizeof(state->buf)) {
        memcpy(state->buf + state->offset, data, length);
        state->offset += length;
    }
}

static void mem_flush_fn(png_structp png_ptr) {
    (void)png_ptr;
}

int main(void) {
    unsigned char width_sym, height_sym, color_type_sym, bit_depth_sym;
    klee_make_symbolic(&width_sym, sizeof(width_sym), "width");
    klee_make_symbolic(&height_sym, sizeof(height_sym), "height");
    klee_make_symbolic(&color_type_sym, sizeof(color_type_sym), "color_type");
    klee_make_symbolic(&bit_depth_sym, sizeof(bit_depth_sym), "bit_depth");

    /* Constrain to valid PNG parameters */
    unsigned int width = (width_sym % MAX_DIM) + 1;
    unsigned int height = (height_sym % MAX_DIM) + 1;
    int color_type = color_type_sym % 7; /* 0,2,3,4,6 are valid */
    int bit_depth;

    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:
            bit_depth = (1 << (bit_depth_sym % 5)); /* 1,2,4,8,16 */
            break;
        case PNG_COLOR_TYPE_PALETTE:
            bit_depth = (1 << (bit_depth_sym % 4)); /* 1,2,4,8 */
            break;
        default:
            bit_depth = (bit_depth_sym & 1) ? 16 : 8;
            break;
    }

    struct write_state state = { .offset = 0 };

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return 0;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 0;
    }

    png_set_write_fn(png_ptr, &state, mem_write_fn, mem_flush_fn);

    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (rowbytes > MAX_ROWBYTES) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 0;
    }

    unsigned char row_data[MAX_ROWBYTES];
    klee_make_symbolic(row_data, sizeof(row_data), "row_data");

    for (unsigned int y = 0; y < height; y++) {
        png_write_row(png_ptr, row_data);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
}
