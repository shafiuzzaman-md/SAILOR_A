#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

// Mirror the minimal types used in harness/colorspace.c
#ifndef FZ_COLORSPACE_LAB
#define FZ_COLORSPACE_LAB 1
#endif
#ifndef FZ_COLORSPACE_INDEXED
#define FZ_COLORSPACE_INDEXED 2
#endif

typedef struct fz_context fz_context; // opaque

typedef struct fz_colorspace {
    int type;
    int n;
    union {
        struct {
            int high;
            unsigned char *lookup;
            struct fz_colorspace *base;
        } indexed;
    } u;
} fz_colorspace;

// Declaration of target function
void fz_clamp_color(fz_context *ctx, fz_colorspace *cs, const float *in, float *out);

int main(void)
{
    // Allocate colorspace and set to non-LAB, non-INDEXED to hit the else-branch
    fz_colorspace *cs = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));
    /* klee_assert removed */
    cs->type = 0; // neither FZ_COLORSPACE_LAB nor FZ_COLORSPACE_INDEXED

    // Set n larger than the provided input length to cause OOB read on in[i]
    cs->n = 4; // will read in[1], in[2], in[3]

    // Allocate input with only 1 float; make its content symbolic
    float *in = (float *)malloc(sizeof(float) * 1);
    /* klee_assert removed */
    { static const unsigned char in_buf_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(in, in_buf_data, (sizeof(float) * 1 < sizeof(in_buf_data)) ? sizeof(float) * 1 : sizeof(in_buf_data)); };

    // Allocate output buffer with n elements (avoid write-side OOB)
    float *out = (float *)malloc(sizeof(float) * cs->n);
    /* klee_assert removed */
    { static const unsigned char out_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(out, out_buf_data, (sizeof(float) * cs->n < sizeof(out_buf_data)) ? sizeof(float) * cs->n : sizeof(out_buf_data)); };

    // Call entry/vulnerable function directly
    fz_clamp_color(NULL, cs, in, out);
    return 0;
}
