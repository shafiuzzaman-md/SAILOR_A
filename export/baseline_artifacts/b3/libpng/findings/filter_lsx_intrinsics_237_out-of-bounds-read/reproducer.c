#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal png typedefs */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Minimal LSX vector and intrinsics stubs to simulate behavior */
#ifndef __m128i
typedef struct { unsigned char b[16]; } __m128i;
#endif

static inline __m128i __lsx_vldrepl_w(const void *p, int off)
{
    const uint8_t *addr = (const uint8_t *)p + off;
    /* This 4-byte load is what triggers the OOB when only 3 bytes are available */
    uint32_t w = *(const uint32_t *)addr;  /* ASan will instrument this */
    __m128i r;
    for (int i = 0; i < 4; i++) {
        r.b[i*4 + 0] = ((uint8_t *)&w)[0];
        r.b[i*4 + 1] = ((uint8_t *)&w)[1];
        r.b[i*4 + 2] = ((uint8_t *)&w)[2];
        r.b[i*4 + 3] = ((uint8_t *)&w)[3];
    }
    return r;
}

static inline __m128i __lsx_vldrepl_b(const void *p, int off)
{
    const uint8_t *addr = (const uint8_t *)p + off;
    uint8_t v = *addr;  /* 1-byte load */
    __m128i r;
    for (int i = 0; i < 16; i++) r.b[i] = v;
    return r;
}

static inline __m128i __lsx_vsrli_b(__m128i v, int imm)
{
    __m128i r;
    for (int i = 0; i < 16; i++) r.b[i] = (uint8_t)(v.b[i] >> (imm & 7));
    return r;
}

static inline __m128i __lsx_vadd_b(__m128i a, __m128i b)
{
    __m128i r;
    for (int i = 0; i < 16; i++) r.b[i] = (uint8_t)(a.b[i] + b.b[i]);
    return r;
}

static inline __m128i __lsx_vavg_bu(__m128i a, __m128i b)
{
    __m128i r;
    for (int i = 0; i < 16; i++) {
        unsigned sum = (unsigned)a.b[i] + (unsigned)b.b[i] + 1u;
        r.b[i] = (uint8_t)(sum >> 1);
    }
    return r;
}

static inline void __lsx_vstelm_h(__m128i v, void *p, int off, int idx)
{
    uint8_t *dst = (uint8_t *)p + off;
    /* store selected halfword (2 bytes) */
    dst[0] = v.b[idx * 2 + 0];
    dst[1] = v.b[idx * 2 + 1];
}

static inline void __lsx_vstelm_b(__m128i v, void *p, int off, int idx)
{
    uint8_t *dst = (uint8_t *)p + off;
    dst[0] = v.b[idx & 15];
}

/* Vulnerable function re-declared to reproduce the bug */
void png_read_filter_row_avg3_lsx(png_row_info *row_info, png_byte *row,
                                  const png_byte *prev_row)
{
    size_t n = row_info->rowbytes;
    png_byte *nxt = row;
    const png_byte *prev_nxt = prev_row;
    __m128i vec_0, vec_1, vec_2;

    /* OOB read when n == 3 (rowbytes == 3): reads 4 bytes from a 3-byte row */
    vec_0 = __lsx_vldrepl_w(nxt, 0);
    vec_1 = __lsx_vldrepl_w(prev_nxt, 0);
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
    }
}

int main(void)
{
    /* Set rowbytes to 3 to force the vulnerable 4-byte load on a 3-byte buffer */
    png_row_info info;
    info.rowbytes = 3;  /* width=1, bpp=3 => 3 row bytes */

    png_byte *row = (png_byte *)malloc(3);       /* Only 3 bytes allocated */
    png_byte *prev = (png_byte *)malloc(4);      /* Make prev at least 4 bytes to avoid its OOB */
    if (!row || !prev) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }

    /* Initialize buffers with some data */
    row[0] = 0x11; row[1] = 0x22; row[2] = 0x33;
    prev[0] = 0x44; prev[1] = 0x55; prev[2] = 0x66; prev[3] = 0x77;

    /* Call the vulnerable function - ASan should report heap-buffer-overflow */
    png_read_filter_row_avg3_lsx(&info, row, prev);

    /* Clean up */
    free(row);
    free(prev);
    return 0;
}
