#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal typedefs mimicking libpng types */
typedef uint8_t png_byte;
typedef uint32_t png_uint_32;

/* Stub out png_debug */
#define png_debug(level, msg) do { (void)(level); } while (0)

/* --- Stub NEON intrinsics (portable, no real NEON required) --- */
typedef struct { png_uint_32 lane[4]; } uint32x4_t;

static inline uint32x4_t vld1q_dup_u32(const png_uint_32 *p)
{
    uint32x4_t v;
    v.lane[0] = *p;
    v.lane[1] = *p;
    v.lane[2] = *p;
    v.lane[3] = *p;
    return v;
}

static inline uint32x4_t vld1q_lane_u32(const png_uint_32 *p, uint32x4_t v, int lane)
{
    if (lane >= 0 && lane < 4) v.lane[lane] = *p;
    return v;
}

static inline void vst1q_u32(void *ptr, uint32x4_t v)
{
    /* Write 16 bytes starting at ptr. This will trigger ASan if ptr is OOB. */
    png_uint_32 *out = (png_uint_32 *)ptr;
    out[0] = v.lane[0];
    out[1] = v.lane[1];
    out[2] = v.lane[2];
    out[3] = v.lane[3];
}

/* --- Vulnerable function (copied and minimally adapted) --- */
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
        cur = vld1q_dup_u32 (riffled_palette + *(sp - 3));
        cur = vld1q_lane_u32(riffled_palette + *(sp - 2), cur, 1);
        cur = vld1q_lane_u32(riffled_palette + *(sp - 1), cur, 2);
        cur = vld1q_lane_u32(riffled_palette + *(sp - 0), cur, 3);
        /* Out-of-bounds write happens here for certain row_width values */
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
    /* Choose a row_width that is >=5 and not a multiple of 4 to trigger the bug */
    const png_uint_32 row_width = 5; /* Minimal trigger */

    /* Prepare a riffled palette (256 entries), contents don't matter for OOB */
    png_uint_32 *riffled_palette = (png_uint_32 *)malloc(256 * sizeof(png_uint_32));
    if (!riffled_palette) return 1;
    for (int i = 0; i < 256; i++) riffled_palette[i] = 0x11223300u | (png_uint_32)i;

    /* Source row: add 3 bytes of left padding so sp-3..sp-0 are in-bounds
     * even on the final (buggy) iteration when i = 4. */
    size_t src_alloc_sz = row_width + 3; /* 3 bytes padding at the start */
    png_byte *src_alloc = (png_byte *)malloc(src_alloc_sz);
    if (!src_alloc) return 1;
    memset(src_alloc, 0, src_alloc_sz);
    png_byte *src_start = src_alloc + 3; /* logical start of indices */

    /* Fill padding and indices with valid palette indices */
    src_start[-3] = 10; /* padding byte used when i=4 */
    src_start[-2] = 11;
    src_start[-1] = 12;
    for (png_uint_32 i = 0; i < row_width; i++) src_start[i] = (png_byte)(20 + i);

    /* Destination row: allocate exactly row_width*4 bytes so the buggy store
     * goes before the start and ASan reports an out-of-bounds write. */
    size_t dst_sz = row_width * 4; /* RGBA8 */
    png_byte *dst = (png_byte *)malloc(dst_sz);
    if (!dst) return 1;
    memset(dst, 0xCC, dst_sz);

    /* Set pointers as expected by the function: both point to the last byte */
    const png_byte *ssp = src_start + (row_width - 1); /* last index byte */
    png_byte *ddp = dst + (dst_sz - 1);               /* last byte of dest row */

    /* Call the vulnerable function: this should trigger ASan */
    (void)png_target_do_expand_palette_rgba8_neon(riffled_palette, row_width, &ssp, &ddp);

    /* Cleanup (not reached if ASan aborts on error) */
    free(dst);
    free(src_alloc);
    free(riffled_palette);

    return 0;
}
