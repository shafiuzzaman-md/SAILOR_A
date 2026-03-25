/*
 * B1 Human-written harness: mupdf buffer operations
 * Targets: fz_new_buffer, fz_append_data, fz_terminate_buffer,
 *          fz_resize_buffer, fz_clone_buffer, fz_buffer_storage
 */
#include <mupdf/fitz.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_DATA_SIZE 64

int main(void) {
    unsigned char data[MAX_DATA_SIZE];
    unsigned char init_cap;
    unsigned char resize_cap;
    unsigned char ops;

    klee_make_symbolic(data, sizeof(data), "buffer_data");
    klee_make_symbolic(&init_cap, sizeof(init_cap), "init_capacity");
    klee_make_symbolic(&resize_cap, sizeof(resize_cap), "resize_capacity");
    klee_make_symbolic(&ops, sizeof(ops), "operations");

    size_t capacity = (init_cap % 64) + 1;  /* 1-64 */

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) return 0;

    fz_try(ctx) {
        fz_buffer *buf = fz_new_buffer(ctx, capacity);

        /* Append symbolic data */
        fz_append_data(ctx, buf, data, sizeof(data));

        /* Various operations based on symbolic flags */
        if (ops & 0x01) {
            fz_terminate_buffer(ctx, buf);
        }
        if (ops & 0x02) {
            size_t new_cap = (resize_cap % 128) + 1;
            fz_resize_buffer(ctx, buf, new_cap);
        }
        if (ops & 0x04) {
            fz_buffer *clone = fz_clone_buffer(ctx, buf);
            fz_drop_buffer(ctx, clone);
        }
        if (ops & 0x08) {
            unsigned char *bdata;
            size_t blen = fz_buffer_storage(ctx, buf, &bdata);
            (void)blen;
        }
        if (ops & 0x10) {
            fz_append_data(ctx, buf, data, sizeof(data));
            fz_terminate_buffer(ctx, buf);
        }
        if (ops & 0x20) {
            fz_clear_buffer(ctx, buf);
            fz_append_data(ctx, buf, data, 8);
        }

        fz_drop_buffer(ctx, buf);
    }
    fz_catch(ctx) {
        /* Expected */
    }

    fz_drop_context(ctx);
    return 0;
}
