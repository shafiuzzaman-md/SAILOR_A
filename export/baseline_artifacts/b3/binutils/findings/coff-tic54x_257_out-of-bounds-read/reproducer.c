#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for BFD types used in the vulnerable code. */
typedef struct bfd bfd;

typedef struct reloc_howto_type {
  unsigned int type;
  const char *name;
  /* Real BFD has many more fields; we only need one to dereference. */
} reloc_howto_type;

typedef struct arelent {
  reloc_howto_type *howto;
} arelent;

struct internal_reloc {
  int r_symndx;        /* -1 indicates a TI "internal relocation" (banked). */
  unsigned int r_type; /* relocation type to match in howto table */
};

/* HOWTO_BANK value used when r_symndx == -1. In TIC54X this is 6. */
#define HOWTO_BANK 6

/* Create a minimal tic54x_howto_table with 13 entries (0..12). The only
   entry of interest is at index 12. There is NO banked counterpart, so
   adding HOWTO_BANK goes past the end of the table. */
static reloc_howto_type tic54x_howto_table[13] = {
  { 0,  "HOWTO0" },
  { 1,  "HOWTO1" },
  { 2,  "HOWTO2" },
  { 3,  "HOWTO3" },
  { 4,  "HOWTO4" },
  { 5,  "HOWTO5" },
  { 6,  "HOWTO6" },
  { 7,  "HOWTO7" },
  { 8,  "HOWTO8" },
  { 9,  "HOWTO9" },
  { 10, "HOWTO10" },
  { 11, "HOWTO11" },
  /* The problematic relocation lives here (e.g., R_RELLONG / STAB). */
  { 12, "HOWTO12_ONLY_UNBANKED" }
};

/* Vulnerable function from bfd/coff-tic54x.c simplified to essentials. */
static void tic54x_lookup_howto(bfd *abfd, arelent *internal, struct internal_reloc *dst) {
  (void)abfd; /* unused */
  unsigned i;
  int bank = (dst->r_symndx == -1) ? HOWTO_BANK : 0;

  for (i = 0; i < sizeof tic54x_howto_table / sizeof tic54x_howto_table[0]; i++) {
    if (tic54x_howto_table[i].type == dst->r_type) {
      /* BUG: no bounds check when adding bank, can point past end */
      internal->howto = tic54x_howto_table + i + bank;
      return;
    }
  }
  internal->howto = NULL;
}

int main(void) {
  /* Craft relocation that matches index 12 and forces banked selection. */
  struct internal_reloc rel;
  rel.r_symndx = -1; /* triggers banked selection (HOWTO_BANK added) */
  rel.r_type   = 12; /* matches tic54x_howto_table[12] */

  arelent genrel;
  genrel.howto = NULL;

  /* Call the vulnerable function: it will compute &table[12 + 6] = &table[18],
     which is past the end of the 13-element array. */
  tic54x_lookup_howto(NULL, &genrel, &rel);

  if (!genrel.howto) {
    fprintf(stderr, "howto lookup failed unexpectedly\n");
    return 1;
  }

  /* Dereference the invalid howto pointer to trigger ASan out-of-bounds read
     on the global array redzone. */
  volatile unsigned int t = genrel.howto->type; /* OOB read here */
  printf("Read howto->type = %u (this line may not print if ASan aborts)\n", t);

  return 0;
}
