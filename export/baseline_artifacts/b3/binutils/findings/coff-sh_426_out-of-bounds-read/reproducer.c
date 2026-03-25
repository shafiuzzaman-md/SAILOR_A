#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal stub type definitions to mirror the relevant BFD/COFF types. */
typedef unsigned long long bfd_vma;

struct bfd { int dummy; };

struct bfd_section {
  bfd_vma vma;
  struct bfd_section *output_section; /* Not used in this reproducer */
  struct bfd *owner;                  /* Not used in this reproducer */
};

typedef struct bfd_section asection;

struct internal_reloc {
  unsigned long r_type;
};

struct coff_link_hash_entry { int dummy; };

struct internal_syment {
  int n_scnum;
  unsigned long n_value;
};

/* Simplified reloc_howto_type with only the field we dereference. */
typedef struct {
  int pc_relative;
} reloc_howto_type;

/* Stubs/macros to satisfy the function body without pulling real BFD headers. */
#define BFD_ASSERT(x) ((void)0)
#define R_SH_IMAGEBASE 0x1234u
struct pe_d { struct { unsigned long long ImageBase; } pe_opthdr; };
#define pe_data(x) ((struct pe_d*)(0))

/* Define a small howto table to simulate the real one. */
#define SH_COFF_HOWTO_COUNT 4
static reloc_howto_type sh_coff_howtos[SH_COFF_HOWTO_COUNT] = {
  { .pc_relative = 0 },
  { .pc_relative = 0 },
  { .pc_relative = 0 },
  { .pc_relative = 0 }
};

/* Vulnerable function adapted from bfd/coff-sh.c (coff_sh_rtype_to_howto). */
static reloc_howto_type *
coff_sh_rtype_to_howto(struct bfd *abfd, asection *sec,
                       struct internal_reloc *rel,
                       struct coff_link_hash_entry *h,
                       struct internal_syment *sym,
                       bfd_vma *addendp)
{
  reloc_howto_type *howto;

  /* Vulnerable index: no bounds check on rel->r_type. */
  howto = sh_coff_howtos + rel->r_type;

  *addendp = 0;

  /* OOB read happens here when rel->r_type >= SH_COFF_HOWTO_COUNT. */
  if (howto->pc_relative)
    *addendp += sec->vma;

  if (sym != NULL && sym->n_scnum == 0 && sym->n_value != 0)
  {
    BFD_ASSERT(h != NULL);
  }

  if (howto->pc_relative)
  {
    *addendp -= 4;
    if (sym != NULL && sym->n_scnum != 0)
      *addendp -= sym->n_value;
  }

  if (rel->r_type == R_SH_IMAGEBASE)
    *addendp -= pe_data(sec->output_section->owner)->pe_opthdr.ImageBase;

  return howto;
}

int main(void)
{
  /* Set up minimal inputs to reach the vulnerable path. */
  struct bfd dummy_bfd = {0};
  asection sec = {0};
  sec.vma = 0x1000;

  /* Craft a relocation with r_type exactly one past the end of the table. */
  struct internal_reloc rel;
  rel.r_type = SH_COFF_HOWTO_COUNT; /* OOB index */

  /* Keep sym/h NULL so we don't need additional setup. */
  struct internal_syment *sym = NULL;
  struct coff_link_hash_entry *h = NULL;
  bfd_vma addend = 0;

  /* This call will perform an out-of-bounds read of sh_coff_howtos. */
  reloc_howto_type *ret = coff_sh_rtype_to_howto(&dummy_bfd, &sec, &rel, h, sym, &addend);

  /* Prevent optimizing away the call result. */
  if (ret == (reloc_howto_type *)0xdeadbeef) {
    printf("Impossible branch: %p\n", (void*)ret);
  }

  printf("If ASan is enabled, an out-of-bounds read should have been reported.\n");
  return 0;
}
