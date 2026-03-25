#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stub types to mirror the BFD structures used */
typedef struct asymbol {
  int dummy;
} asymbol;

typedef struct asection {
  asymbol symbol;
  uintptr_t vma;
  struct asection *output_section;
  uintptr_t output_offset;
} asection;

typedef struct arelent {
  uintptr_t address;
  asymbol **sym_ptr_ptr;
  void *howto;
} arelent;

struct internal_reloc {
  long r_vaddr;
  long r_symndx;
  long r_type;
};

typedef struct bfd {
  int *conv_table;
  int conv_table_size;
} bfd;

/* Global absolute section pointer stub */
static asection abs_section;
static asection *bfd_abs_section_ptr = &abs_section;

/* Stubs for functions/macros used in the vulnerable function */
static inline int *obj_convert(bfd *abfd) { return abfd->conv_table; }
static inline int obj_conv_table_size(bfd *abfd) { return abfd->conv_table_size; }

static void _bfd_error_handler(const char *fmt, ...) {
  /* No-op: we don't need actual error handling for the repro */
  (void)fmt;
}

#define CALC_ADDEND(abfd, ptr, reloc, relent) do { (void)(abfd); (void)(ptr); (void)(reloc); (void)(relent); } while (0)

static void tic4x_lookup_howto(bfd *abfd, arelent *relent, struct internal_reloc *rel) {
  (void)abfd; (void)relent; (void)rel;
  /* No-op stub */
}

/* Vulnerable function, adapted from bfd/coff-tic4x.c */
static void
tic4x_reloc_processing(arelent *relent,
                       struct internal_reloc *reloc,
                       asymbol **symbols,
                       bfd *abfd,
                       asection *section)
{
  asymbol *ptr;

  relent->address = (uintptr_t)reloc->r_vaddr;

  if (reloc->r_symndx != -1 && symbols != NULL)
    {
      if (reloc->r_symndx < 0 || reloc->r_symndx >= obj_conv_table_size(abfd))
        {
          _bfd_error_handler("warning: illegal symbol index %ld in relocs", reloc->r_symndx);
          relent->sym_ptr_ptr = &bfd_abs_section_ptr->symbol;
          ptr = NULL;
        }
      else
        {
          /* Vulnerable access: no bounds check that mapped index is within 'symbols' array. */
          relent->sym_ptr_ptr = (symbols + obj_convert(abfd)[reloc->r_symndx]);
          ptr = *(relent->sym_ptr_ptr); /* OOB read happens here if index is out of range */
        }
    }
  else
    {
      relent->sym_ptr_ptr = &section->symbol;
      ptr = *(relent->sym_ptr_ptr);
    }

  CALC_ADDEND(abfd, ptr, *reloc, relent);

  /* Use section->vma as in original function. */
  relent->address -= section->vma;

  tic4x_lookup_howto(abfd, relent, reloc);
}

int main(void) {
  /* Prepare a tiny 'symbols' array with 1 entry. */
  size_t symbols_count = 1;
  asymbol **symbols = (asymbol **)malloc(symbols_count * sizeof(asymbol *));
  if (!symbols) {
    perror("malloc symbols");
    return 1;
  }
  asymbol sym0 = {0};
  symbols[0] = &sym0;

  /* Create a bfd with a conversion table of size 1 that maps index 0 -> 100, far beyond symbols size. */
  bfd abfd = {0};
  abfd.conv_table_size = 1;
  abfd.conv_table = (int *)malloc(sizeof(int) * abfd.conv_table_size);
  if (!abfd.conv_table) {
    perror("malloc conv_table");
    return 1;
  }
  abfd.conv_table[0] = 100; /* Mapped index is out of bounds for 'symbols' (size 1). */

  /* Minimal section stub. */
  asection sec = {0};
  sec.vma = 0;
  sec.output_section = &sec;
  sec.output_offset = 0;

  /* Internal relocation: r_symndx within conv table, but mapped index is invalid. */
  struct internal_reloc reloc = {0};
  reloc.r_vaddr = 0x1234;
  reloc.r_symndx = 0; /* Valid wrt conv_table_size (1), but mapped to 100. */
  reloc.r_type = 0;

  arelent relent = {0};

  /* This call will perform an out-of-bounds read from the 'symbols' array. */
  tic4x_reloc_processing(&relent, &reloc, symbols, &abfd, &sec);

  /* Clean up (not reached if ASan aborts on OOB). */
  free(abfd.conv_table);
  free(symbols);

  /* Print something to avoid optimizing away code paths (though -O0 is used). */
  printf("Done. address=%p\n", (void *)relent.address);
  return 0;
}
