#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Minimal stub/type redeclarations to mirror the vulnerable logic. */

typedef unsigned long long bfd_vma;
typedef unsigned char bfd_byte;

#define SYMNMLEN 8

struct coff_link_hash_entry { int dummy; };

struct internal_syment {
    struct {
        struct {
            uint32_t _n_zeroes;
            uint32_t _n_offset;
        } _n_n;
        char _n_name[SYMNMLEN];
    } _n;
    bfd_vma n_value;
};

typedef struct asection {
    struct outsec { bfd_vma vma; } *output_section;
    bfd_vma output_offset;
    bfd_vma vma;
    unsigned int reloc_count;
} asection;

typedef struct bfd {
    struct coff_link_hash_entry **sym_hashes;  /* what obj_coff_sym_hashes() returns */
    char *strings;
    size_t strings_len;
    /* Only fields used by our stub. */
} bfd;

static inline struct coff_link_hash_entry **obj_coff_sym_hashes(bfd *abfd) {
    return abfd->sym_hashes;
}
static inline const char *obj_coff_strings(bfd *abfd) {
    return abfd->strings;
}
static inline size_t obj_coff_strings_len(bfd *abfd) {
    return abfd->strings_len;
}

typedef struct {
    unsigned short r_type;
    int r_symndx;
    uint32_t r_vaddr;
} RELOC;

typedef int bfd_reloc_status_type; /* unused in the stub */
typedef struct { int dummy; } reloc_howto_type;

/* Stub that always returns a non-NULL howto to keep execution going past the OOB read site. */
static reloc_howto_type *bfd_coff_rtype_to_howto(bfd *input_bfd,
                                                asection *input_section,
                                                RELOC *rel,
                                                struct coff_link_hash_entry *h,
                                                struct internal_syment *sym,
                                                bfd_vma *addend)
{
    static reloc_howto_type dummy;
    (void)input_bfd; (void)input_section; (void)rel; (void)h; (void)sym; (void)addend;
    return &dummy;
}

/* A self-contained replica of the vulnerable portion of coff_mcore_relocate_section. */
static bool coff_mcore_relocate_section(bfd *input_bfd,
                                        bfd *output_bfd, /* unused, stubbed */
                                        asection *input_section,
                                        asection **sections,
                                        bfd_byte *contents,
                                        RELOC *relocs,
                                        struct internal_syment *syms)
{
    (void)output_bfd; /* Not needed for this reproducer */

    RELOC *rel = relocs;
    RELOC *relend = rel + input_section->reloc_count;

    for (; rel < relend; rel++) {
        long symndx;
        struct internal_syment *sym;
        bfd_vma val;
        bfd_vma addend;
        bfd_reloc_status_type rstat;
        bfd_byte *loc;
        unsigned short r_type = rel->r_type;
        reloc_howto_type *howto = NULL;
        struct coff_link_hash_entry *h;
        const char *my_name;
        char buf[SYMNMLEN + 1];

        (void)rstat; (void)r_type; (void)my_name; (void)buf; /* silence unused warnings */

        symndx = rel->r_symndx;
        loc = contents + rel->r_vaddr - input_section->vma;
        (void)loc;

        if (symndx == -1) {
            h = NULL;
            sym = NULL;
        } else {
            /* Vulnerable out-of-bounds read of sym_hashes using unchecked symndx. */
            h = obj_coff_sym_hashes(input_bfd)[symndx];
            /* Potential OOB pointer computation for symbols array as well. */
            sym = syms + symndx;
        }

        addend = 0;
        /* Keep executing to ensure the above read occurs before any early return. */
        howto = bfd_coff_rtype_to_howto(input_bfd, input_section, rel, h, sym, &addend);
        if (howto == NULL)
            return false;

        /* The rest of the real function would continue, but it's unnecessary for the OOB read. */
    }

    return true;
}

int main(void)
{
    /* Craft a minimal environment: a tiny sym_hashes array but a large symndx in relocs. */
    bfd *input_bfd = (bfd *)calloc(1, sizeof(bfd));
    if (!input_bfd) return 1;

    /* Allocate only 1 entry for sym_hashes so any index >= 1 is OOB. */
    input_bfd->sym_hashes = (struct coff_link_hash_entry **)calloc(1, sizeof(struct coff_link_hash_entry *));
    if (!input_bfd->sym_hashes) return 1;
    input_bfd->strings = (char *)"";
    input_bfd->strings_len = 0;

    asection *input_section = (asection *)calloc(1, sizeof(asection));
    if (!input_section) return 1;
    input_section->vma = 0;
    input_section->reloc_count = 1; /* single relocation */

    /* Contents buffer (not relevant to the bug, but needed for code path). */
    bfd_byte *contents = (bfd_byte *)calloc(16, 1);
    if (!contents) return 1;

    /* Relocation with an out-of-range symbol index. */
    RELOC relocs[1];
    relocs[0].r_type = 0;
    relocs[0].r_vaddr = 0;
    relocs[0].r_symndx = 1000; /* deliberately far beyond the size of sym_hashes (size 1) */

    /* Minimal symbols/sections arrays (size 1) that will also be OOB if used. */
    struct internal_syment *syms = (struct internal_syment *)calloc(1, sizeof(struct internal_syment));
    if (!syms) return 1;
    asection **sections = (asection **)calloc(1, sizeof(asection *));
    if (!sections) return 1;

    /* Call the vulnerable logic. AddressSanitizer should report heap-buffer-overflow
       due to the out-of-bounds read of sym_hashes[1000]. */
    bool ok = coff_mcore_relocate_section(input_bfd, NULL, input_section, sections, contents, relocs, syms);

    /* Avoid optimizing away */
    printf("coff_mcore_relocate_section returned: %s\n", ok ? "true" : "false");

    return 0;
}
