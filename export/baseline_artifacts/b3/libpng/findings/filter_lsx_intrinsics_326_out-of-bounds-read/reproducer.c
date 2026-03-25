#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal type aliases matching libpng expectations */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Very small stub implementation of LSX intrinsics sufficient to
 * compile and exercise the first block of png_read_filter_row_paeth3_lsx.
 * These stubs intentionally perform real memory accesses so ASan can
 * detect the out-of-bounds read.
 */
typedef struct { uint8_t b[16]; } __m128i;

static inline __m128i __lsx_vldrepl_w(const void *p, int imm)
{
    /* Intentionally performs a 4-byte load from p, which will be OOB if only
     * 3 bytes are valid, matching the vulnerability scenario.
     */
    (void)imm;
    __m128i v;
    memset(&v, 0, sizeof(v));
    /* This 4-byte load is what we want ASan to flag when p points to a 3-byte buffer */
    uint32_t w = *(const uint32_t*)p; /* OOB read by 1 byte when only 3 bytes valid */
    v.b[0] = (uint8_t)(w & 0xFF);
    v.b[1] = (uint8_t)((w >> 8) & 0xFF);
    v.b[2] = (uint8_t)((w >> 16) & 0xFF);
    v.b[3] = (uint8_t)((w >> 24) & 0xFF);
    return v;
}

static inline __m128i __lsx_vadd_b(__m128i a, __m128i b)
{
    __m128i r;
    for (int i = 0; i < 16; ++i) r.b[i] = (uint8_t)(a.b[i] + b.b[i]);
    return r;
}

static inline void __lsx_vstelm_h(__m128i v, void *dst, int imm1, int imm2)
{
    (void)imm1; (void)imm2;
    uint8_t *d = (uint8_t*)dst;
    d[0] = v.b[0];
    d[1] = v.b[1];
}

static inline void __lsx_vstelm_b(__m128i v, void *dst, int imm1, int imm2)
{
    (void)imm1; (void)imm2;
    uint8_t *d = (uint8_t*)dst;
    d[0] = v.b[2];
}

/* Reimplementation of the vulnerable function body up to (and including)
 * the first block where the OOB read happens. The rest is not needed for
 * triggering the bug.
 */
__attribute__((noinline))
void png_read_filter_row_paeth3_lsx(png_row_info *row_info, png_byte *row, const png_byte *prev_row)
{
    size_t n = row_info->rowbytes;
    png_byte *nxt = row;
    const png_byte *prev_nxt = prev_row;
    __m128i vec_a, vec_b, vec_d;

    /* This read is safe in the reproducer because we over-allocate row. */
    vec_a = __lsx_vldrepl_w(nxt, 0);

    /* This is the vulnerable read: it reads 4 bytes even if only 3 are valid. */
    vec_b = __lsx_vldrepl_w(prev_nxt, 0); /* OOB by 1 when rowbytes == 3 */
    prev_nxt += 3;

    vec_d = __lsx_vadd_b(vec_a, vec_b);
    __lsx_vstelm_h(vec_d, nxt, 0, 0);
    nxt += 2;
    __lsx_vstelm_b(vec_d, nxt, 0, 2);
    nxt += 1;
    n -= 3;

    /* Stop here; the bug is already triggered by the read above. */
    (void)n; (void)prev_nxt;
}

int main(void)
{
    /* rowbytes = 3 (e.g., width=1, 3 bytes per pixel RGB) */
    png_row_info info;
    info.rowbytes = 3;

    /* Allocate row with 4 bytes so the first load from row is not OOB.
     * The vulnerability we want to show is the 4-byte load from prev_row. */
    png_byte *row = (png_byte*)malloc(4);
    if (!row) return 1;

    /* Allocate prev_row with exactly 3 bytes so the 4-byte load overruns by 1. */
    png_byte *prev_row = (png_byte*)malloc(3);
    if (!prev_row) return 1;

    /* Initialize buffers with deterministic data */
    for (int i = 0; i < 4; ++i) row[i] = (png_byte)(0x10 + i);
    for (int i = 0; i < 3; ++i) prev_row[i] = (png_byte)(0x20 + i);

    /* Call the vulnerable function; ASan should report a 1-byte OOB read
     * at the __lsx_vldrepl_w(prev_row, 0) inside. */
    png_read_filter_row_paeth3_lsx(&info, row, prev_row);

    /* Cleanup */
    free(row);
    free(prev_row);

    return 0;
}
