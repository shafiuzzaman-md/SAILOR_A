#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal stub types to mirror the BFD/COFF structures used by
   coff_arm_adjust_symndx in bfd/coff-arm.c */

typedef struct asection {
    struct asection *output_section;
} asection;

enum { ARM_26 = 1, ARM_26D = 2 };

enum bfd_link_hash_type { bfd_link_hash_defined = 1, bfd_link_hash_defweak = 2 };

struct bfd_link_hash_def {
    asection *section;
};

struct bfd_link_hash_root {
    enum bfd_link_hash_type type;
    union {
        struct bfd_link_hash_def def;
    } u;
};

struct coff_link_hash_entry {
    struct bfd_link_hash_root root;
};

typedef struct bfd {
    struct coff_link_hash_entry **sym_hashes;   /* Array of symbol hash ptrs */
    size_t sym_hashes_count;                    /* Number of entries in array */
} bfd;

struct bfd_link_info { int dummy; };

struct internal_reloc {
    int r_type;
    long r_symndx;  /* Index into obj_coff_sym_hashes(ibfd) */
};

#define obj_coff_sym_hashes(abfd) ((abfd)->sym_hashes)

/* Reimplementation of the vulnerable function from coff-arm.c */
static bool coff_arm_adjust_symndx(bfd *obfd, struct bfd_link_info *info,
                                   bfd *ibfd, asection *sec,
                                   struct internal_reloc *irel,
                                   bool *adjustedp)
{
    (void)obfd; (void)info; /* Unused in this reproducer */

    if (irel->r_type == ARM_26) {
        struct coff_link_hash_entry *h;
        /* Vulnerable out-of-bounds read if r_symndx is invalid. */
        h = obj_coff_sym_hashes(ibfd)[irel->r_symndx];
        if (h != NULL
            && (h->root.type == bfd_link_hash_defined
                || h->root.type == bfd_link_hash_defweak)
            && h->root.u.def.section->output_section == sec->output_section)
        {
            irel->r_type = ARM_26D;
        }
    }
    *adjustedp = false;
    return true;
}

int main(void)
{
    /* Set up a fake input BFD with a very small sym_hashes array. */
    bfd *ibfd = (bfd *)calloc(1, sizeof(bfd));
    if (!ibfd) {
        perror("calloc ibfd");
        return 1;
    }

    /* Allocate a tiny symbol-hash array with 1 entry (all NULL). */
    ibfd->sym_hashes_count = 1;
    ibfd->sym_hashes = (struct coff_link_hash_entry **)calloc(ibfd->sym_hashes_count,
                                                             sizeof(struct coff_link_hash_entry *));
    if (!ibfd->sym_hashes) {
        perror("calloc sym_hashes");
        return 1;
    }

    /* Output section placeholder. */
    asection sec = { 0 };
    sec.output_section = &sec; /* Non-NULL, self-referential for simplicity. */

    /* Craft a reloc that uses ARM_26 and an out-of-bounds r_symndx. */
    struct internal_reloc reloc;
    reloc.r_type = ARM_26;
    reloc.r_symndx = 8; /* Deliberately out of bounds: >= sym_hashes_count (1). */

    bool adjusted = false;

    /* Calling the vulnerable function triggers the OOB read on the sym_hashes array. */
    (void)coff_arm_adjust_symndx(NULL, NULL, ibfd, &sec, &reloc, &adjusted);

    /* If ASan didn't abort yet, print a message. */
    printf("Completed call (ASan should have reported an out-of-bounds read).\n");

    /* Cleanup (unlikely to be reached if ASan triggers). */
    free(ibfd->sym_hashes);
    free(ibfd);

    return 0;
}
