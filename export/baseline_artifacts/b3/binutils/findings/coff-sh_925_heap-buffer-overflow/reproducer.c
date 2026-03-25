#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by the vulnerable code path. */
typedef uint64_t bfd_vma;
typedef struct bfd { int dummy; } bfd;

typedef struct asection {
    bfd_vma vma;
    struct asection* output_section;
    bfd_vma output_offset;
} asection;

typedef struct internal_reloc {
    bfd_vma r_vaddr;
    int r_type;
    int r_symndx;
    bfd_vma r_offset;
} internal_reloc;

/* Relocation and symbol class constants (minimal). */
#define R_SH_USES    1
#define R_SH_PCDISP  2
#define R_SH_COUNT   3
#define C_EXT        2

/* Simplified coff_section_data holder and accessor. */
typedef struct coff_secdata {
    void *relocs;
    unsigned char *contents;
} coff_secdata;

static coff_secdata global_secdata;
static inline coff_secdata* coff_section_data(bfd *abfd, asection *sec) {
    (void)abfd; (void)sec;
    return &global_secdata;
}

/* Minimal bfd_put_16 that writes 2 bytes to the provided buffer. */
__attribute__((noinline))
static void bfd_put_16(bfd *abfd, bfd_vma val, unsigned char *addr) {
    (void)abfd;
    /* Write in big-endian just like BFD often does for SH; endianness is not
       important for triggering the overflow. */
    addr[0] = (unsigned char)((val >> 8) & 0xff);
    addr[1] = (unsigned char)(val & 0xff);
}

/* A very small struct to hold the only symbol field we care about here. */
typedef struct { int n_sclass; } simple_sym;

/*
 * A trimmed-down version of the vulnerable portion of sh_relax_section.
 * It executes the same write as in coff-sh.c:925 without checking that there
 * are at least 2 bytes remaining in the section buffer.
 */
__attribute__((noinline))
static void sh_relax_section(bfd *abfd,
                             asection *sec,
                             unsigned char *contents,
                             internal_reloc *internal_relocs,
                             size_t nrelocs) {
    (void)nrelocs;

    /* Set local variables to mimic context in the real function. */
    internal_reloc *irel = &internal_relocs[0];
    internal_reloc *irelfn = &internal_relocs[0];
    simple_sym sym;
    sym.n_sclass = 1; /* Not C_EXT, so we hit the first bfd_put_16 path. */

    /* foff must be within [-0x1000, 0x1000). Choose values so foff == 0. */
    bfd_vma symval = irel->r_vaddr - sec->vma + sec->output_section->vma + sec->output_offset + 4;
    bfd_vma foff = (bfd_vma)((int64_t)symval - (int64_t)(irel->r_vaddr - sec->vma + sec->output_section->vma + sec->output_offset + 4));
    if ((int64_t)foff < -0x1000LL || (int64_t)foff >= 0x1000LL) {
        /* Not expected in this reproducer. */
        return;
    }

    /* Emulate the code around lines 909-934 from coff-sh.c */
    coff_section_data(abfd, sec)->relocs = internal_relocs;
    coff_section_data(abfd, sec)->contents = contents;

    /* Change the reloc type and sym index as in the real code. */
    irel->r_type = R_SH_PCDISP;
    irel->r_symndx = irelfn->r_symndx;

    if (sym.n_sclass != C_EXT) {
        /* Vulnerable write: no bounds check before writing 2 bytes. */
        bfd_put_16(abfd, (bfd_vma)0xb000 | (((int64_t)foff >> 1) & 0xfff),
                   contents + irel->r_vaddr - sec->vma);
    } else {
        bfd_put_16(abfd, (bfd_vma)0xb000, contents + irel->r_vaddr - sec->vma);
    }
}

int main(void) {
    /* Allocate a small section buffer on the heap. */
    const size_t section_size = 8; /* Intentionally small. */
    unsigned char *contents = (unsigned char*)malloc(section_size);
    if (!contents) {
        perror("malloc");
        return 1;
    }
    memset(contents, 0xCC, section_size);

    /* Set up section and output section so that sec->vma == 0. */
    asection out_sec = {0};
    asection sec = {0};
    sec.vma = 0;
    sec.output_section = &out_sec;
    sec.output_offset = 0;
    out_sec.vma = 0;

    /* Create a single relocation entry. */
    internal_reloc rels[1];
    memset(rels, 0, sizeof(rels));

    /* Critical part: set r_vaddr to the LAST BYTE of the buffer. */
    rels[0].r_vaddr = (bfd_vma)(section_size - 1); /* points to byte N-1 */
    rels[0].r_type = R_SH_USES;  /* initial type before conversion */
    rels[0].r_symndx = 0;
    rels[0].r_offset = 0;

    bfd dummy_bfd = {0};

    /* This call will perform a 16-bit write starting at contents + (N-1),
       overflowing the heap buffer by 1 byte and triggering ASan. */
    sh_relax_section(&dummy_bfd, &sec, contents, rels, 1);

    /* If ASan didn't already abort, clean up. */
    free(contents);
    return 0;
}
