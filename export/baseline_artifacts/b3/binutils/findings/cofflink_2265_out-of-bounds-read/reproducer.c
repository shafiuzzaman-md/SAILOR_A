#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Minimal stand-ins for the COFF internal types used in the vulnerable code. */
struct internal_lineno {
    union {
        long l_symndx;
        long l_paddr;
    } l_addr;
    unsigned short l_lnno;
};

struct internal_syment {
    /* Only fields referenced by the real code; content doesn't matter here. */
    uint32_t n_type;
    uint32_t n_sclass;
    uint32_t n_numaux;
    /* Pad to make memcpy sizable so ASan has a larger read to check. */
    uint8_t pad[64];
};

union internal_auxent {
    struct {
        struct {
            struct {
                uint32_t x_lnnoptr;
            } x_fcn;
        } x_fcnary;
    } x_sym;
};

/* Flaginfo stand-in: carries the outsyms buffer and symbol index map. */
struct flaginfo_s {
    char *outsyms;
    char *linenos;
    long *sym_indices;
};

/* Stubs for the BFD swap/utility functions referenced by the snippet. */
static void bfd_coff_swap_lineno_in(void *input_bfd, const void *src, struct internal_lineno *iline) {
    (void)input_bfd; (void)src;
    /* Craft values to enter the vulnerable path: l_lnno == 0 and valid sym index 0 */
    iline->l_lnno = 0;
    iline->l_addr.l_symndx = 0;
}

static void bfd_coff_swap_sym_in(void *output_bfd, const void *src, struct internal_syment *dst) {
    (void)output_bfd;
    /* This memcpy causes ASan to check reads from "src". If src points before
       the allocated buffer (underflow), ASan will report an OOB read. */
    memcpy(dst, src, sizeof(*dst));
}

static unsigned long obj_raw_syment_count(void *input_bfd) {
    (void)input_bfd;
    /* Say we have at least 1 raw symbol so the bound check passes. */
    return 1UL;
}

/* Reimplementation of the vulnerable logic around lines 2231-2293 from _bfd_coff_final_link. */
static void trigger_vulnerable_path(void) {
    /* Parameters shaping the vulnerable expression: outsyms + ((indx - syment_base) * osymesz) */
    const long syment_base = 1;    /* Make base larger than indx to force negative (underflow) */
    const long osymesz     = 16;   /* Size of one output symbol entry */
    const size_t outsz     = 32;   /* Small outsyms buffer */

    struct flaginfo_s flaginfo;
    flaginfo.outsyms = (char *)malloc(outsz);
    flaginfo.linenos = (char *)malloc(16);
    flaginfo.sym_indices = (long *)malloc(sizeof(long) * 1);

    if (!flaginfo.outsyms || !flaginfo.linenos || !flaginfo.sym_indices) {
        fprintf(stderr, "alloc failed\n");
        exit(1);
    }

    /* Fill buffers to make ASan's redzones evident. */
    memset(flaginfo.outsyms, 0x41, outsz);
    memset(flaginfo.linenos, 0x42, 16);

    /* Map raw symbol 0 to output index 0 (indx). With syment_base=1 this
       yields (indx - syment_base) = -1, so pointer goes before outsyms. */
    flaginfo.sym_indices[0] = 0;

    /* Minimal setup for the loop context. */
    char linebytes[4] = {0};
    char *eline = linebytes;
    char *elineend = linebytes + sizeof(linebytes); /* single iteration */

    void *input_bfd = NULL;
    void *output_bfd = NULL;
    long offset = 0;

    for (; eline < elineend; eline += sizeof(linebytes)) {
        struct internal_lineno iline;
        bfd_coff_swap_lineno_in(input_bfd, eline, &iline);

        if (iline.l_lnno != 0) {
            iline.l_addr.l_paddr += offset;
        } else if (iline.l_addr.l_symndx >= 0 &&
                   (unsigned long)iline.l_addr.l_symndx < obj_raw_syment_count(input_bfd)) {
            long indx = flaginfo.sym_indices[iline.l_addr.l_symndx];
            if (indx < 0) {
                /* not taken in this reproducer */
            } else {
                struct internal_syment is;
                union internal_auxent ia;
                (void)ia; /* not used, but keep to mirror structure */

                /* Vulnerable access: if indx < syment_base, the subtraction is negative
                   and the resulting pointer is before flaginfo.outsyms, causing OOB read. */
                const char *sym_ptr = flaginfo.outsyms + ((indx - syment_base) * osymesz);
                /* This call performs a memcpy from sym_ptr, which is out-of-bounds. */
                bfd_coff_swap_sym_in(output_bfd, sym_ptr, &is);
            }
            /* Mirror assignment in the original code. */
            /* iline.l_addr.l_symndx = indx; */
        }
    }

    /* Clean up (unreached if ASan aborts on the OOB read). */
    free(flaginfo.outsyms);
    free(flaginfo.linenos);
    free(flaginfo.sym_indices);
}

int main(void) {
    fprintf(stderr, "Triggering out-of-bounds read in simulated _bfd_coff_final_link path...\n");
    trigger_vulnerable_path();
    fprintf(stderr, "If you see this message, ASan did not detect the OOB read (unexpected).\n");
    return 0;
}
