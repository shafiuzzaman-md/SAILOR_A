#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

/* Minimal stub types to model BFD structures used by coff_pe_aarch64_relocate_section. */

typedef uint64_t bfd_vma;
typedef unsigned char bfd_byte;

typedef struct asection {
    struct asection *output_section;
    bfd_vma vma;
    bfd_vma size;
    bfd_vma output_offset;
    unsigned int reloc_count;
} asection;

typedef struct bfd {
    void *tdata; /* We'll hang our hash-array off this pointer */
} bfd;

struct bfd_link_info { int dummy; };

/* Reloc and symbol entry stubs. */
struct internal_reloc {
    long r_symndx;
    uint16_t r_type;
    uint64_t r_vaddr;
};

struct internal_syment {
    bfd_vma n_value;
};

/* Hash structures used by the code. */
enum { bfd_link_hash_defined = 1 };

struct bfd_link_hash_def {
    asection *section;
    bfd_vma value;
};

struct bfd_link_hash_root {
    int type;
    union {
        struct bfd_link_hash_def def;
    } u;
};

struct coff_link_hash_entry {
    struct bfd_link_hash_root root;
};

/* Constants for reloc types. */
#define IMAGE_REL_ARM64_ADDR32    1
#define IMAGE_REL_ARM64_ADDR64    2
#define IMAGE_REL_ARM64_ABSOLUTE  3

/* Stubs for external helpers/macros. */
static int bfd_link_relocatable(struct bfd_link_info *info) { (void)info; return 0; }
static int bfd_is_und_section(asection *sec) { (void)sec; return 0; }
static int discarded_section(asection *sec) { (void)sec; return 0; }

static struct coff_link_hash_entry **obj_coff_sym_hashes(bfd *abfd) {
    return (struct coff_link_hash_entry **)abfd->tdata;
}

static unsigned long obj_raw_syment_count(bfd *abfd) {
    (void)abfd;
    /* Intentionally small to demonstrate that r_symndx is out of range. */
    return 1UL;
}

/* Error handler stub (not expected to be reached before the bug triggers). */
static void _bfd_error_handler(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

/* Vulnerable function, adapted from bfd/coff-aarch64.c */
static bool
coff_pe_aarch64_relocate_section (bfd *output_bfd,
                                  struct bfd_link_info *info,
                                  bfd *input_bfd,
                                  asection *input_section,
                                  bfd_byte *contents,
                                  struct internal_reloc *relocs,
                                  struct internal_syment *syms,
                                  asection **sections)
{
    (void)output_bfd;
    (void)contents;
    struct internal_reloc *rel;
    struct internal_reloc *relend;

    if (bfd_link_relocatable (info))
        return true;

    rel = relocs;
    relend = rel + input_section->reloc_count;

    for (; rel < relend; rel++)
    {
        long symndx;
        struct coff_link_hash_entry *h;
        bfd_vma sym_value;
        asection *sec = NULL;
        uint64_t dest_vma;

        /* skip trivial relocations */
        if (rel->r_type == IMAGE_REL_ARM64_ADDR32
            || rel->r_type == IMAGE_REL_ARM64_ADDR64
            || rel->r_type == IMAGE_REL_ARM64_ABSOLUTE)
            continue;

        symndx = rel->r_symndx;
        /* BUG: symndx is used before bounds check. This will read out-of-bounds
           when symndx is larger than the provided syms array. */
        sym_value = syms[symndx].n_value; /* OOB read here */

        h = obj_coff_sym_hashes (input_bfd)[symndx]; /* Also OOB (later) */

        if (h && h->root.type == bfd_link_hash_defined)
        {
            sec = h->root.u.def.section;
            sym_value = h->root.u.def.value;
        }
        else
        {
            sec = sections[symndx]; /* Also OOB (later) */
        }

        if (!sec)
            continue;

        if (bfd_is_und_section (sec))
            continue;

        if (discarded_section (sec))
            continue;

        dest_vma = sec->output_section->vma + sec->output_offset + sym_value;
        (void)dest_vma; /* suppress unused warning if we get this far */

        if (symndx < 0
            || (unsigned long) symndx >= obj_raw_syment_count (input_bfd))
            continue; /* Bounds check happens too late. */
    }

    return true;
}

int main(void) {
    /* Prepare minimal structures. */
    bfd input_bfd_obj = {0};
    bfd *input_bfd = &input_bfd_obj;
    bfd *output_bfd = NULL;
    struct bfd_link_info link_info = {0};

    /* Set up a section with one relocation. */
    asection out_sec = { .output_section = NULL, .vma = 0, .size = 0, .output_offset = 0, .reloc_count = 0 };
    asection in_sec = { .output_section = &out_sec, .vma = 0, .size = 16, .output_offset = 0, .reloc_count = 1 };

    /* One relocation with a very large symbol index to trigger OOB. */
    struct internal_reloc relocs[1];
    relocs[0].r_type = 4;      /* Not filtered by the trivial-reloc check. */
    relocs[0].r_symndx = 1000000; /* Deliberately out of range. */
    relocs[0].r_vaddr = 0;

    /* Provide a very small symbols array (size 1). */
    struct internal_syment *syms = (struct internal_syment *)malloc(sizeof(struct internal_syment) * 1);
    syms[0].n_value = 0;

    /* Provide a very small sections array (size 1). */
    asection **sections = (asection **)malloc(sizeof(asection *) * 1);
    sections[0] = &in_sec; /* Any non-NULL to pass earlier checks if reached. */

    /* Provide a very small hash array (size 1) hung off input_bfd->tdata. */
    struct coff_link_hash_entry **hashes = (struct coff_link_hash_entry **)calloc(1, sizeof(*hashes));
    input_bfd->tdata = hashes;

    /* Dummy contents buffer. */
    bfd_byte contents[16] = {0};

    /* This call should trigger the OOB read on syms[symndx] before bounds check. */
    (void)coff_pe_aarch64_relocate_section(output_bfd, &link_info, input_bfd, &in_sec,
                                           contents, relocs, syms, sections);

    /* Clean up (not reached if ASan aborts on OOB). */
    free(hashes);
    free(sections);
    free(syms);

    return 0;
}
