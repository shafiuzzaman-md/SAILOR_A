#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal libpng-like typedefs/macros */
typedef unsigned char png_byte;
typedef uint32_t png_uint_32;
typedef struct { size_t rowbytes; } png_row_info;
#define PNG_UNUSED(x) (void)(x)

/* Minimal LSX vector type */
typedef struct { uint8_t b[16]; } __m128i;

/* A volatile sink to force side effects so the compiler can't DCE the call */
static volatile uint32_t g_sink = 0;

/* Stub implementations of the LSX intrinsics used by the vulnerable function. */
static __m128i __lsx_vldrepl_w(const void *p, int imm)
{
    (void)imm; /* immediate is unused in this stub */
    __m128i r;
    /* Unconditionally perform a 4-byte load starting at p, emulating the
       LSX behavior that triggers the OOB read when only 3 bytes are available. */
    const uint8_t *cp = (const uint8_t *)p;
    /* Read 4 individual bytes to avoid alignment UB; this still trips ASan. */
    uint32_t w = (uint32_t)cp[0]
               | ((uint32_t)cp[1] << 8)
               | ((uint32_t)cp[2] << 16)
               | ((uint32_t)cp[3] << 24);
    /* Replicate the 32-bit word across all 4 lanes (16 bytes total). */
    for (int lane = 0; lane < 4; ++lane) {
        r.b[lane * 4 + 0] = (uint8_t)(w & 0xFF);
        r.b[lane * 4 + 1] = (uint8_t)((w >> 8) & 0xFF);
        r.b[lane * 4 + 2] = (uint8_t)((w >> 16) & 0xFF);
        r.b[lane * 4 + 3] = (uint8_t)((w >> 24) & 0xFF);
    }
    /* Create a visible side-effect so the call can't be optimized away. */
    g_sink ^= w;
    return r;
}

static __m128i __lsx_vadd_b(__m128i a, __m128i b)
{
    __m128i r;
    for (int i = 0; i < 16; ++i) r.b[i] = (uint8_t)(a.b[i] + b.b[i]);
    return r;
}

static void __lsx_vstelm_h(__m128i v, void *p, int imm, int idx)
{
    (void)imm; (void)idx; /* In this reproducer we only need the simplest form */
    uint8_t *dp = (uint8_t *)p;
    dp[0] = v.b[0];
    dp[1] = v.b[1];
}

static void __lsx_vstelm_b(__m128i v, void *p, int imm, int idx)
{
    (void)imm;
    uint8_t *dp = (uint8_t *)p;
    /* Store the requested byte index (as used by the original code with idx=2). */
    if (idx < 0) idx = 0;
    if (idx > 15) idx = 15;
    *dp = v.b[idx];
}

/* Vulnerable function from loongarch/filter_lsx_intrinsics.c */
void png_read_filter_row_sub3_lsx(png_row_info *row_info, png_byte *row,
    const png_byte *prev_row)
{
    size_t n = row_info->rowbytes;
    png_uint_32 tmp;
    png_byte *nxt = row;
    __m128i vec_0, vec_1;

    PNG_UNUSED(prev_row);
    PNG_UNUSED(tmp);

    /* OOB read when n == 3 (1-pixel-wide RGB row): loads 4 bytes starting at nxt */
    vec_0 = __lsx_vldrepl_w(nxt, 0);
    nxt += 3;
    n -= 3;

    while (n >= 3)
    {
        vec_1 = __lsx_vldrepl_w(nxt, 0);
        vec_1 = __lsx_vadd_b(vec_1, vec_0);
        __lsx_vstelm_h(vec_1, nxt, 0, 0);
        vec_0 = vec_1;
        nxt += 2;
        __lsx_vstelm_b(vec_1, nxt, 0, 2);
        nxt += 1;
        n -= 3;
    }

    row = nxt - 3;
    while (n--)
    {
        *nxt = (png_byte)(*nxt + *row++);
        nxt++;
    }
}

int main(void)
{
    /* Allocate exactly 3 bytes to model a 1-pixel-wide RGB row (rowbytes == 3). */
    png_byte *row = (png_byte *)malloc(3);
    if (!row) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    row[0] = 0x11; row[1] = 0x22; row[2] = 0x33;

    png_row_info info;
    info.rowbytes = 3; /* 1 pixel, 3 bytes per pixel */

    /* prev_row is unused in this filter. */
    png_read_filter_row_sub3_lsx(&info, row, NULL);

    /* If ASan didn't abort yet (it should), clean up. */
    free(row);
    return 0;
}
