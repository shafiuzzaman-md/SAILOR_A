#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal png types */
typedef uint8_t png_byte;
typedef uint32_t png_uint_32;

typedef struct {
    png_byte red;
    png_byte green;
    png_byte blue;
} png_color; /* sizeof(png_color) == 3 */

/* Stub out png_debug used by the function */
#define png_debug(level, msg) ((void)0)

/* NEON intrinsic stubs (portable no-ops): */
typedef struct { uint8_t val[3][8]; } uint8x8x3_t;
static inline uint8x8x3_t vld3_dup_u8(const uint8_t *ptr) {
    /* No-op stub; argument evaluation still happens before call */
    (void)ptr;
    uint8x8x3_t r; memset(&r, 0, sizeof(r)); return r;
}
static inline uint8x8x3_t vld3_lane_u8(const uint8_t *ptr, uint8x8x3_t v, int lane) {
    (void)ptr; (void)lane; return v;
}
static inline void vst3_u8(void *dst, uint8x8x3_t v) {
    (void)dst; (void)v; /* No write to keep things simple */
}

/* Vulnerable function (verbatim logic) */
static png_uint_32
png_target_do_expand_palette_rgb8_neon(const png_color *paletteIn,
    png_uint_32 row_width, const png_byte **ssp, png_byte **ddp)
{
    /* TODO: This case is VERY dangerous: */
    const png_byte *palette = (const png_byte *)paletteIn;

    const png_uint_32 pixels_per_chunk = 8;
    png_uint_32 i;

    png_debug(1, "in png_do_expand_palette_rgb8_neon");

    if (row_width <= pixels_per_chunk)
        return 0;

    /* Seeking this back by 8 pixels x 3 bytes. */
    *ddp = *ddp - ((pixels_per_chunk * sizeof(png_color)) - 1);

    for (i = 0; i < row_width; i += pixels_per_chunk)
    {
        uint8x8x3_t cur;
        const png_byte *sp = *ssp - i;
        png_byte *dp = *ddp - ((i << 1) + i);
        cur = vld3_dup_u8(palette + sizeof(png_color) * (*(sp - 7)));
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 6)), cur, 1);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 5)), cur, 2);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 4)), cur, 3);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 3)), cur, 4);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 2)), cur, 5);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 1)), cur, 6);
        cur = vld3_lane_u8(palette + sizeof(png_color) * (*(sp - 0)), cur, 7);
        vst3_u8((void *)dp, cur);
    }

    if (i != row_width)
    {
        /* Remove the amount that wasn't processed. */
        i -= pixels_per_chunk;
    }

    /* Decrement output pointers. */
    *ssp = *ssp - i;
    *ddp = *ddp - ((i << 1) + i);
    return i;
}

int main(void)
{
    /* Choose a row_width > 8 and not a multiple of 8 to exercise the bug on the second iteration */
    const png_uint_32 row_width = 9; /* triggers i = 0 and i = 8 iterations */

    /* Allocate a small input row (palette indices). */
    png_byte *row = (png_byte *)malloc(row_width);
    if (!row) { perror("malloc row"); return 1; }
    for (png_uint_32 i = 0; i < row_width; ++i) row[i] = (png_byte)(i & 0xFF);

    /* Prepare ssp to point 7 bytes into the row so the first iteration is in-bounds,
       but the second iteration will read before the start of the allocation. */
    const png_byte *ssp_local = row + 7;

    /* Allocate a palette of 256 entries (RGB8). */
    png_color *palette = (png_color *)malloc(256 * sizeof(png_color));
    if (!palette) { perror("malloc palette"); return 1; }
    for (int i = 0; i < 256; ++i) {
        palette[i].red   = (png_byte)i;
        palette[i].green = (png_byte)(255 - i);
        palette[i].blue  = (png_byte)(i ^ 0x55);
    }

    /* Output buffer; we'll make it large and set ddp somewhere in the middle. */
    size_t out_size = 3 * row_width + 128;
    png_byte *out = (png_byte *)malloc(out_size);
    if (!out) { perror("malloc out"); return 1; }
    memset(out, 0, out_size);

    png_byte *ddp_local = out + 64; /* roomy headroom for the function's backward adjustments */

    /* Call the vulnerable function. This should trigger an ASan OOB read on the second loop. */
    png_target_do_expand_palette_rgb8_neon(palette, row_width, &ssp_local, &ddp_local);

    /* Cleanup (may not be reached if ASan aborts on error) */
    free(out);
    free(palette);
    free(row);

    printf("Done (if you see this, ASan may not have been enabled).\n");
    return 0;
}
