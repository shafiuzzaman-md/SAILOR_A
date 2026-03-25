#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal typedefs to match the vulnerable function's signature */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

/* No-op debug macro */
#define png_debug(level, msg) do { (void)(level); } while (0)

/* Stub NEON-like types and intrinsics so this compiles on non-ARM targets */
typedef struct { uint32_t v[4]; } uint32x4_t;

static inline uint32x4_t vld1q_dup_u32(const uint32_t *p)
{
    uint32x4_t r;
    r.v[0] = *p;
    r.v[1] = *p;
    r.v[2] = *p;
    r.v[3] = *p;
    return r;
}

static inline uint32x4_t vld1q_lane_u32(const uint32_t *p, uint32x4_t r, int lane)
{
    if (lane >= 0 && lane < 4) r.v[lane] = *p;
    return r;
}

static inline void vst1q_u32(void *dst, uint32x4_t r)
{
    memcpy(dst, r.v, sizeof(r.v));
}

/* Vulnerable function (adapted from the provided source context) */
static png_uint_32
png_target_do_expand_palette_rgba8_neon(const png_uint_32 *riffled_palette,
    png_uint_32 row_width, const png_byte **ssp, png_byte **ddp)
{
    const png_uint_32 pixels_per_chunk = 4;
    png_uint_32 i;

    png_debug(1, "in png_do_expand_palette_rgba8_neon");

    if (row_width < pixels_per_chunk)
        return 0;

    /* This function originally gets the last byte of the output row.
     * The NEON part writes forward from a given position, so we have
     * to seek this back by 4 pixels x 4 bytes.
     */
    *ddp = *ddp - ((pixels_per_chunk * sizeof(png_uint_32)) - 1);

    for (i = 0; i < row_width; i += pixels_per_chunk)
    {
        uint32x4_t cur;
        const png_byte *sp = *ssp - i;
        png_byte *dp = *ddp - (i << 2);
        /* BUG: When row_width is not a multiple of 4 and >= 5, on the
         * second iteration (i == 4), sp points before the start of the
         * source row, so (sp - 3) underflows and is an OOB read.
         */
        cur = vld1q_dup_u32 (riffled_palette + *(sp - 3));
        cur = vld1q_lane_u32(riffled_palette + *(sp - 2), cur, 1);
        cur = vld1q_lane_u32(riffled_palette + *(sp - 1), cur, 2);
        cur = vld1q_lane_u32(riffled_palette + *(sp - 0), cur, 3);
        vst1q_u32((void *)dp, cur);
    }
    if (i != row_width)
    {
        /* Remove the amount that wasn't processed. */
        i -= pixels_per_chunk;
    }

    /* Decrement output pointers. */
    *ssp = *ssp - i;
    *ddp = *ddp - (i << 2);
    return i;
}

int main(void)
{
    /* row_width chosen to be 5: not a multiple of 4 and >= 5 to hit the bug */
    const png_uint_32 row_width = 5;

    /* Allocate a small source row of exactly 5 bytes.
     * ASan will place a left redzone before this allocation so reading
     * 3 bytes before will trigger a heap-buffer-underflow.
     */
    png_byte *src = (png_byte *)malloc(row_width);
    if (!src) return 1;

    /* Fill with valid palette indices [0..255] */
    for (png_uint_32 i = 0; i < row_width; ++i)
        src[i] = (png_byte)(i & 0xFF);

    /* Prepare destination buffer for RGBA8 output: row_width * 4 bytes */
    png_byte *dst = (png_byte *)malloc(row_width * 4);
    if (!dst) return 1;

    /* Prepare a simple riffled palette of 256 entries */
    png_uint_32 *riffled_palette = (png_uint_32 *)malloc(256 * sizeof(png_uint_32));
    if (!riffled_palette) return 1;
    for (int i = 0; i < 256; ++i)
        riffled_palette[i] = (png_uint_32)(0xFF000000u | (i << 16) | (i << 8) | i);

    /* Set ssp to point at the last byte of the source row (as expected by the function) */
    const png_byte *ssp = src + row_width - 1;  /* src[4] */

    /* Set ddp to point at the last byte of the output row */
    png_byte *ddp = dst + (row_width * 4) - 1;  /* dst[19] */

    /* Call the vulnerable function: on the second iteration (i == 4), it will read *(sp - 3)
     * where sp == ssp - 4 == src - 0, so (sp - 3) == src - 3 (OOB read before buffer).
     */
    (void)png_target_do_expand_palette_rgba8_neon(riffled_palette, row_width, &ssp, &ddp);

    /* Cleanup (we likely won't reach here if ASan aborts on error) */
    free(riffled_palette);
    free(dst);
    free(src);

    return 0;
}
