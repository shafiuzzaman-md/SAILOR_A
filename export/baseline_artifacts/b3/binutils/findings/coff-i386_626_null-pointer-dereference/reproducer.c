#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal stand-ins for BFD/COFF structures and constants needed to
   reproduce the NULL pointer dereference in coff_i386_rtype_to_howto. */

typedef uint64_t bfd_vma;

typedef struct asection asection;

typedef struct bfd {
    asection *sections; /* Head of section list */
} bfd;

struct asection {
    asection *next;
    asection *output_section;
    bfd_vma vma;
    bfd *owner;
};

/* Relocation types (only what we need). */
enum {
    R_SECREL32 = 0x000F, /* Arbitrary value; only equality check matters */
    R_IMAGEBASE = 0x0010 /* Unused here */
};

/* Simplified relocation entry. */
struct arelent {
    int r_type;
    bfd_vma r_vaddr;
};

/* Simplified COFF internal symbol. */
struct internal_syment {
    int n_scnum;   /* Section number (1-based) */
    bfd_vma n_value;
};

/* Minimal hash entry stand-in. We keep it so the code matches the flow. */
struct coff_link_hash_entry {
    struct {
        int type;
        union {
            struct {
                asection *section;
            } def;
        } u;
    } root;
};

enum {
    bfd_link_hash_defined = 1,
    bfd_link_hash_defweak = 2
};

/* Vulnerable function reimplemented with only the relevant logic path. */
static void coff_i386_rtype_to_howto(
    bfd *abfd,
    struct arelent *rel,
    struct internal_syment *sym,
    asection *sec,                 /* Unused in our path */
    bfd_vma *addendp,
    struct coff_link_hash_entry *h /* We'll pass NULL to force the else path */
) {
    (void)sec; /* Suppress unused warning */

    if (rel->r_type == R_SECREL32 && sym != NULL) {
        bfd_vma osect_vma;

        if (h && (h->root.type == bfd_link_hash_defined ||
                  h->root.type == bfd_link_hash_defweak)) {
            osect_vma = h->root.u.def.section->output_section->vma;
        } else {
            /* Vulnerable path: walk section list using sym->n_scnum without bounds check. */
            asection *s;
            int i;

            for (s = abfd->sections, i = 1; i < sym->n_scnum; i++)
                s = s->next;  /* If n_scnum exceeds number of sections, s becomes NULL. */

            /* NULL dereference here when s == NULL. */
            osect_vma = s->output_section->vma; /* Crash: s is NULL */
        }

        *addendp -= osect_vma;
    }
}

int main(void) {
    /* Build a fake bfd with a single section in the list. */
    bfd fake_bfd = {0};
    asection sec1 = {0};

    sec1.next = NULL;               /* Only one section in the list */
    sec1.output_section = &sec1;    /* Typical self-reference in simple cases */
    sec1.vma = 0x1000;
    sec1.owner = &fake_bfd;

    fake_bfd.sections = &sec1;      /* Head points to the single section */

    /* Prepare a relocation of type R_SECREL32. */
    struct arelent rel = {0};
    rel.r_type = R_SECREL32;
    rel.r_vaddr = 0;

    /* sym->n_scnum is 1-based. Setting it to 2 with only 1 section ensures
       the loop walks past the end, leaving s == NULL after the loop. */
    struct internal_syment sym = {0};
    sym.n_scnum = 2; /* Malformed input: exceeds actual section count */
    sym.n_value = 0;

    bfd_vma addend = 0;

    /* h == NULL so the code will take the else branch that walks sections. */
    coff_i386_rtype_to_howto(&fake_bfd, &rel, &sym, &sec1, &addend, NULL);

    /* We should never reach here; if we do, print something. */
    printf("Unexpectedly survived. addend=%llu\n", (unsigned long long)addend);
    return 0;
}
