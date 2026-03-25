#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Minimal stub types to mimic BFD/COFF structures used in the vulnerable code */
typedef unsigned long bfd_vma;

typedef struct coff_link_hash_entry {
    int indx;
} coff_link_hash_entry;

typedef struct internal_reloc {
    bfd_vma r_vaddr;
    int r_symndx;
} internal_reloc;

typedef struct internal_syment {
    int dummy;
} internal_syment;

/* Minimal bfd and tdata to hold the sym_hashes array */
typedef struct coff_tdata {
    struct coff_link_hash_entry **sym_hashes; /* obj_coff_sym_hashes */
} coff_tdata;

typedef struct bfd {
    coff_tdata *tdata;
} bfd;

/* Minimal section structure */
typedef struct bfd_section {
    bfd_vma vma;
    bfd_vma output_offset;
    struct bfd_section *output_section; /* points to the output section */
    unsigned int reloc_count;            /* number of relocs in this section */
} bfd_section;

/* Pieces of the coff final link info used by the vulnerable code */
typedef struct coff_final_link_section_info {
    struct coff_link_hash_entry **rel_hashes;
} coff_final_link_section_info;

typedef struct coff_final_link_info {
    void *info; /* unused but required for branch selection */
    coff_final_link_section_info *section_info;
    struct internal_syment *internal_syms;
    long *sym_indices;
    void **sec_ptrs; /* unused here */
} coff_final_link_info;

/* Macros/stubs matching the vulnerable code's usage */
#define obj_coff_sym_hashes(abfd) ((abfd)->tdata->sym_hashes)

static inline bool bfd_link_relocatable(void *info) {
    (void)info;
    /* Force the code path that performs the out-of-bounds index */
    return true;
}

/* adjust_symndx callback type (we will pass NULL) */
typedef bool (*adjust_symndx_fn)(bfd *output_bfd, void *link_info, bfd *input_bfd,
                                 bfd_section *sec, internal_reloc *irel, bool *adjusted);

/* A minimized clone of the vulnerable loop from _bfd_coff_final_link. */
bool _bfd_coff_final_link(bfd *output_bfd,
                          bfd *input_bfd,
                          coff_final_link_info *flaginfo,
                          bfd_section *o,
                          internal_reloc *internal_relocs,
                          int target_index,
                          adjust_symndx_fn adjust_symndx)
{
    (void)output_bfd; /* Unused in this minimal reproducer */

    if (bfd_link_relocatable(flaginfo->info)) {
        bfd_vma offset;
        struct internal_reloc *irel, *irelend;
        struct coff_link_hash_entry **rel_hash;

        offset = o->output_section->vma + o->output_offset - o->vma;
        irel = internal_relocs;
        irelend = irel + o->reloc_count;
        rel_hash = (flaginfo->section_info[target_index].rel_hashes
                    + o->output_section->reloc_count);
        for (; irel < irelend; irel++, rel_hash++) {
            struct coff_link_hash_entry *h;
            bool adjusted;

            *rel_hash = NULL;

            /* Adjust the reloc address and symbol index. */
            irel->r_vaddr += offset;

            if (irel->r_symndx == -1)
                continue;

            if (adjust_symndx) {
                if (!(*adjust_symndx)(output_bfd, flaginfo->info, input_bfd, o, irel, &adjusted))
                    return false;
                if (adjusted)
                    continue;
            }

            /* Vulnerable access: no bounds check on r_symndx */
            h = obj_coff_sym_hashes(input_bfd)[irel->r_symndx];

            if (h != NULL) {
                if (h->indx >= 0)
                    irel->r_symndx = h->indx;
                else {
                    *rel_hash = h;
                    h->indx = -2;
                }
            } else {
                long indx;
                indx = flaginfo->sym_indices[irel->r_symndx];
                if (indx != -1)
                    irel->r_symndx = indx;
                else {
                    struct internal_syment *is;
                    const char *name;
                    char buf[9];
                    (void)buf;
                    is = flaginfo->internal_syms + irel->r_symndx;
                    (void)is;
                    name = "dummy"; /* stub */
                    (void)name;
                }
            }
        }
    }

    return true;
}

int main(void) {
    /* Build a minimal input_bfd with a very small sym_hashes array */
    bfd *input_bfd = (bfd *)calloc(1, sizeof(bfd));
    coff_tdata *itd = (coff_tdata *)calloc(1, sizeof(coff_tdata));
    input_bfd->tdata = itd;

    /* Allocate a tiny sym_hashes array (size 1) to maximize OOB chance */
    coff_link_hash_entry **sym_hashes = (coff_link_hash_entry **)calloc(1, sizeof(*sym_hashes));
    sym_hashes[0] = NULL; /* contents don't matter; we just need the array */
    itd->sym_hashes = sym_hashes;

    /* Minimal output_bfd (unused fields) */
    bfd *output_bfd = (bfd *)calloc(1, sizeof(bfd));

    /* Create sections: 'o' is input section; output_section is separate with reloc_count = 0 */
    bfd_section outsec;
    memset(&outsec, 0, sizeof(outsec));
    outsec.vma = 0;
    outsec.output_offset = 0;
    outsec.reloc_count = 0; /* important so rel_hash points to start of array */

    bfd_section osec;
    memset(&osec, 0, sizeof(osec));
    osec.vma = 0;
    osec.output_offset = 0;
    osec.reloc_count = 1; /* one relocation to process */
    osec.output_section = &outsec; /* distinct from osec to avoid rel_hash OOB */

    /* One relocation with an out-of-range r_symndx to trigger OOB read */
    internal_reloc relocs[1];
    relocs[0].r_vaddr = 0;
    relocs[0].r_symndx = 1000000; /* far beyond the size of sym_hashes (1) */

    /* Prepare flaginfo with a valid rel_hashes buffer */
    coff_final_link_info finfo;
    memset(&finfo, 0, sizeof(finfo));
    finfo.info = &finfo; /* non-NULL to take the relocatable branch */

    coff_final_link_section_info sinfo[1];
    memset(sinfo, 0, sizeof(sinfo));

    /* Allocate a small rel_hashes array; size 1 is enough since outsec.reloc_count = 0 */
    coff_link_hash_entry **rel_hashes = (coff_link_hash_entry **)calloc(1, sizeof(*rel_hashes));
    sinfo[0].rel_hashes = rel_hashes;

    finfo.section_info = sinfo;

    /* Allocate tiny arrays for internal_syms and sym_indices to avoid accidental writes */
    finfo.internal_syms = (internal_syment *)calloc(1, sizeof(internal_syment));
    finfo.sym_indices = (long *)calloc(1, sizeof(long));
    finfo.sym_indices[0] = -1; /* default value */

    /* Call the vulnerable function: adjust_symndx = NULL so no adjustment occurs */
    (void)_bfd_coff_final_link(output_bfd, input_bfd, &finfo, &osec, relocs, 0, NULL);

    /* Clean up (not strictly necessary for the repro) */
    free(finfo.sym_indices);
    free(finfo.internal_syms);
    free(rel_hashes);
    free(sinfo);
    free(sym_hashes);
    free(itd);
    free(input_bfd);
    free(output_bfd);

    printf("Done. If built with ASan, an out-of-bounds read should have been reported.\n");
    return 0;
}
