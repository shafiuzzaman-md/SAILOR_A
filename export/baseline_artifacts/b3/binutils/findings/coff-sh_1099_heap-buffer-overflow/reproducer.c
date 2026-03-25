#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declarations to mirror BFD types used by the vulnerable code. */
typedef unsigned char bfd_byte;
typedef unsigned long long bfd_vma;
typedef size_t bfd_size_type;

struct bfd { int dummy; };

/* Internal reloc structure (subset). */
struct internal_reloc {
    int r_type;
    bfd_vma r_vaddr;
    int r_offset;
};

/* Dummy types only to satisfy declarations in the function. */
struct coff_link_hash_entry { int dummy; };
struct internal_syment { int dummy; };

/* Section structure with only fields we need. */
struct coff_section_data_s;
struct asection {
    bfd_size_type size;
    bfd_vma vma;
    size_t reloc_count;
    struct coff_section_data_s *secdata;
};

/* Per-section COFF data used by coff_section_data(). */
struct coff_section_data_s {
    bfd_byte *contents;
    struct internal_reloc *relocs;
};

/* Accessor as in BFD, simplified. */
static struct coff_section_data_s *coff_section_data(struct bfd *abfd, struct asection *sec) {
    (void)abfd;
    return sec->secdata;
}

/* Constants used by the function. */
#define R_SH_ALIGN 1
#define R_SH_CODE  2

/* BFD_ASSERT substitute. */
#define BFD_ASSERT(x) assert(x)

/* bfd_put_16 stub to satisfy linker even though we will not hit that path. */
static void bfd_put_16(struct bfd *abfd, bfd_vma v, bfd_byte *p) {
    (void)abfd;
    /* Store 16-bit value in big-endian just as a placeholder. */
    unsigned short s = (unsigned short)v;
    p[0] = (unsigned char)((s >> 8) & 0xff);
    p[1] = (unsigned char)(s & 0xff);
}

/* Vulnerable function extracted/simplified from bfd/coff-sh.c. */
static bool sh_relax_delete_bytes(struct bfd *abfd,
                                  struct asection *sec,
                                  bfd_vma addr,
                                  int count)
{
    bfd_byte *contents;
    struct internal_reloc *irel, *irelend;
    struct internal_reloc *irelalign;
    bfd_vma toaddr;
    bfd_byte *esym, *esymend;
    bfd_size_type symesz;
    struct coff_link_hash_entry **sym_hash;
    struct asection *o;

    (void)esym; (void)esymend; (void)symesz; (void)sym_hash; (void)o; /* Unused in this reproducer. */

    contents = coff_section_data(abfd, sec)->contents;

    /* The deletion must stop at the next ALIGN reloc for an alignment
       power larger than the number of bytes we are deleting. */
    irelalign = NULL;
    toaddr = sec->size;

    irel = coff_section_data(abfd, sec)->relocs;
    irelend = irel + sec->reloc_count;
    for (; irel < irelend; irel++) {
        if (irel->r_type == R_SH_ALIGN &&
            irel->r_vaddr - sec->vma > addr &&
            count < (1 << irel->r_offset)) {
            irelalign = irel;
            toaddr = irel->r_vaddr - sec->vma;
            break;
        }
    }

    /* Actually delete the bytes. Vulnerability: no check that addr + count <= toaddr. */
    memmove(contents + addr, contents + addr + count,
            (size_t)(toaddr - addr - count));

    if (irelalign == NULL)
        sec->size -= count;
    else {
        int i;
        #define NOP_OPCODE (0x0009)
        BFD_ASSERT((count & 1) == 0);
        for (i = 0; i < count; i += 2)
            bfd_put_16(abfd, (bfd_vma)NOP_OPCODE, contents + toaddr - count + i);
    }

    /* Adjust all the relocs. In this reproducer reloc_count == 0, so this loop is skipped. */
    for (irel = coff_section_data(abfd, sec)->relocs; irel < irelend; irel++) {
        bfd_vma nraddr, stop;
        bfd_vma start = 0;
        int insn = 0;
        struct internal_syment sym;
        int off, adjust, oinsn;
        bfd_signed_vma voff = 0;
        bool overflow;
        (void)nraddr; (void)stop; (void)start; (void)insn; (void)sym; (void)off; (void)adjust; (void)oinsn; (void)voff; (void)overflow;
    }

    return true;
}

int main(void) {
    /* Set up a small section and contents. */
    size_t sec_size = 16; /* toaddr will be 16 */

    struct coff_section_data_s *sd = (struct coff_section_data_s *)malloc(sizeof(*sd));
    if (!sd) {
        perror("malloc");
        return 1;
    }

    sd->contents = (bfd_byte *)malloc(sec_size);
    if (!sd->contents) {
        perror("malloc");
        return 1;
    }

    /* Fill contents with a pattern. */
    for (size_t i = 0; i < sec_size; i++) sd->contents[i] = (unsigned char)(i & 0xFF);

    /* Allocate at least one reloc entry, but set reloc_count = 0 so the loop is skipped
       while keeping irel/irelend comparisons well-defined. */
    sd->relocs = (struct internal_reloc *)malloc(sizeof(struct internal_reloc) * 1);
    if (!sd->relocs) {
        perror("malloc");
        return 1;
    }

    struct asection sec;
    sec.size = sec_size;
    sec.vma = 0;
    sec.reloc_count = 0; /* ensure no ALIGN reloc found; irelalign stays NULL */
    sec.secdata = sd;

    struct bfd *abfd = NULL; /* Unused by the vulnerable path in this reproducer. */

    /* Craft addr and count so that addr + count > toaddr (sec.size),
       causing (toaddr - addr - count) to underflow to a huge size_t.
       Also make src = contents + addr + count already out-of-bounds. */
    bfd_vma addr = 12;  /* within [0, sec.size) */
    int count = 8;      /* addr + count = 20 > toaddr (16) */

    /* This call triggers the heap-buffer-overflow in memmove. */
    (void)sh_relax_delete_bytes(abfd, &sec, addr, count);

    /* If ASan did not abort (it should), clean up. */
    free(sd->relocs);
    free(sd->contents);
    free(sd);
    return 0;
}
