#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libpng types */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Extremely small stubs for LSX intrinsics used by the function. */
typedef struct { unsigned char b[16]; } __m128i;

static inline __m128i __lsx_vldrepl_w(const void *p, int off)
{
    /* Intentionally perform a 4-byte read to match the buggy behavior. */
    const unsigned char *bp = (const unsigned char *)p + off;
    /* This 4-byte load will trigger ASan if fewer than 4 bytes are addressable. */
    uint32_t v = *(const uint32_t *)bp;
    __m128i out;
    /* Just replicate the 4 bytes across the vector for simplicity. */
    for (int i = 0; i < 16; i += 4) {
        out.b[i + 0] = (unsigned char)(v & 0xFF);
        out.b[i + 1] = (unsigned char)((v >> 8) & 0xFF);
        out.b[i + 2] = (unsigned char)((v >> 16) & 0xFF);
        out.b[i + 3] = (unsigned char)((v >> 24) & 0xFF);
    }
    return out;
}

static inline __m128i __lsx_vldrepl_b(const void *p, int off)
{
    const unsigned char *bp = (const unsigned char *)p + off;
    unsigned char v = *bp; /* 1-byte read */
    __m128i out;
    for (int i = 0; i < 16; ++i) out.b[i] = v;
    return out;
}

static inline __m128i __lsx_vsrli_b(__m128i a, int sh)
{
    __m128i out;
    int s = sh & 7;
    for (int i = 0; i < 16; ++i) out.b[i] = (unsigned char)(a.b[i] >> s);
    return out;
}

static inline __m128i __lsx_vadd_b(__m128i a, __m128i b)
{
    __m128i out;
    for (int i = 0; i < 16; ++i) out.b[i] = (unsigned char)(a.b[i] + b.b[i]);
    return out;
}

static inline __m128i __lsx_vavg_bu(__m128i a, __m128i b)
{
    __m128i out;
    for (int i = 0; i < 16; ++i) {
        unsigned int av = a.b[i];
        unsigned int bv = b.b[i];
        out.b[i] = (unsigned char)(((av + bv) + 1) >> 1);
    }
    return out;
}

static inline void __lsx_vstelm_h(__m128i v, void *p, int /*imm*/, int /*idx*/)
{
    /* Store the first 2 bytes */
    unsigned char *bp = (unsigned char *)p;
    bp[0] = v.b[0];
    bp[1] = v.b[1];
}

static inline void __lsx_vstelm_b(__m128i v, void *p, int /*imm*/, int /*idx*/)
{
    /* Store the 3rd byte to match the usage with idx=2 in the snippet */
    unsigned char *bp = (unsigned char *)p;
    bp[0] = v.b[2];
}

/* Vulnerable function reproduced with the same signature and core logic. */
void png_read_filter_row_avg3_lsx(png_row_info *row_info, png_byte *row,
                                  const png_byte *prev_row)
{
    size_t n = row_info->rowbytes;
    png_byte *nxt = row;
    const png_byte *prev_nxt = prev_row;
    __m128i vec_0, vec_1, vec_2;

    vec_0 = __lsx_vldrepl_w(nxt, 0);
    vec_1 = __lsx_vldrepl_w(prev_nxt, 0); /* OOB 4-byte read when rowbytes == 3 */
    prev_nxt += 3;
    vec_1 = __lsx_vsrli_b(vec_1, 1);
    vec_1 = __lsx_vadd_b(vec_1, vec_0);
    __lsx_vstelm_h(vec_1, nxt, 0, 0);
    nxt += 2;
    __lsx_vstelm_b(vec_1, nxt, 0, 2);
    nxt += 1;
    n -= 3;

    while (n >= 3)
    {
        vec_2 = vec_1;
        vec_0 = __lsx_vldrepl_w(nxt, 0);
        vec_1 = __lsx_vldrepl_w(prev_nxt, 0);
        prev_nxt += 3;

        vec_1 = __lsx_vavg_bu(vec_1, vec_2);
        vec_1 = __lsx_vadd_b(vec_1, vec_0);

        __lsx_vstelm_h(vec_1, nxt, 0, 0);
        nxt += 2;
        __lsx_vstelm_b(vec_1, nxt, 0, 2);
        nxt += 1;
        n -= 3;
    }

    row = nxt - 3;
    while (n--)
    {
        vec_2 = __lsx_vldrepl_b(row, 0);
        row++;
        vec_0 = __lsx_vldrepl_b(nxt, 0);
        vec_1 = __lsx_vldrepl_b(prev_nxt, 0);
        prev_nxt++;

        vec_1 = __lsx_vavg_bu(vec_1, vec_2);
        vec_1 = __lsx_vadd_b(vec_1, vec_0);

        __lsx_vstelm_b(vec_1, nxt, 0, 0);
        nxt++;
    }
}

int main(void)
{
    /* Set up a 1-pixel-wide row with 3 bytes per pixel (rowbytes == 3). */
    png_row_info info;
    info.rowbytes = 3; /* 3 BPP, width == 1 */

    /* Row buffer: allocate 4 bytes so the first 4-byte load from 'row' does NOT trip ASan. */
    png_byte *row = (png_byte *)malloc(4);
    if (!row) return 1;
    memset(row, 0x11, 4);

    /* Previous row buffer: allocate EXACTLY 3 bytes so the 4-byte load overreads by 1. */
    png_byte *prev = (png_byte *)malloc(3);
    if (!prev) return 1;
    memset(prev, 0x22, 3);

    /* This call should trigger an AddressSanitizer heap-buffer-overflow (read of size 4) */
    png_read_filter_row_avg3_lsx(&info, row, prev);

    /* Cleanup (may not be reached if ASan aborts on error). */
    free(prev);
    free(row);

    /* If no ASan, program may just exit. */
    puts("Done");
    return 0;
}
