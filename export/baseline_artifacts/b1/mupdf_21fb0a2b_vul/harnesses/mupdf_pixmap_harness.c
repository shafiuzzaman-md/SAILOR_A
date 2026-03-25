/*
 * B1 Human-written harness: mupdf pixmap operations
 * Targets: fz_new_pixmap, fz_clear_pixmap, fz_clone_pixmap,
 *          fz_invert_pixmap, fz_gamma_pixmap
 */
#include <mupdf/fitz.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

int main(void) {
    unsigned char width_sym, height_sym, n_sym, alpha_sym;
    unsigned char pixel_data[64];

    klee_make_symbolic(&width_sym, sizeof(width_sym), "width");
    klee_make_symbolic(&height_sym, sizeof(height_sym), "height");
    klee_make_symbolic(&n_sym, sizeof(n_sym), "n_components");
    klee_make_symbolic(&alpha_sym, sizeof(alpha_sym), "alpha");
    klee_make_symbolic(pixel_data, sizeof(pixel_data), "pixel_data");

    /* Constrain to reasonable values */
    int w = (width_sym % 16) + 1;    /* 1-16 */
    int h = (height_sym % 16) + 1;   /* 1-16 */
    int n = (n_sym % 4) + 1;         /* 1-4 */
    int alpha = alpha_sym & 1;        /* 0 or 1 */

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) return 0;

    fz_try(ctx) {
        fz_colorspace *cs = NULL;
        switch (n) {
            case 1: cs = fz_device_gray(ctx); break;
            case 3: cs = fz_device_rgb(ctx); break;
            case 4: cs = fz_device_cmyk(ctx); break;
            default: cs = fz_device_gray(ctx); n = 1; break;
        }

        fz_pixmap *pix = fz_new_pixmap(ctx, cs, w, h, NULL, alpha);

        /* Fill with symbolic data */
        unsigned char *samples = fz_pixmap_samples(ctx, pix);
        size_t stride = fz_pixmap_stride(ctx, pix);
        size_t total = stride * h;
        if (total <= sizeof(pixel_data)) {
            memcpy(samples, pixel_data, total);
        }

        /* Operations */
        fz_clear_pixmap(ctx, pix);
        fz_invert_pixmap(ctx, pix);
        fz_gamma_pixmap(ctx, pix, 2.2f);

        fz_pixmap *clone = fz_clone_pixmap(ctx, pix);
        fz_drop_pixmap(ctx, clone);
        fz_drop_pixmap(ctx, pix);
    }
    fz_catch(ctx) {
        /* Expected for invalid parameters */
    }

    fz_drop_context(ctx);
    return 0;
}
