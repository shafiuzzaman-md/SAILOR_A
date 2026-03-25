#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

/* Minimal stand-ins for BFD/COFF types used by the vulnerable code path */
typedef unsigned long long bfd_vma;

typedef struct bfd { int dummy; } bfd;

typedef struct asection {
  bfd_vma vma;
  struct asection *output_section;
} asection;

/* Relocation status types */
typedef enum {
  bfd_reloc_ok = 0,
  bfd_reloc_outofrange = 1,
  bfd_reloc_overflow = 2
} bfd_reloc_status_type;

/* Howto stub */
typedef struct reloc_howto_type {
  const char *name;
} reloc_howto_type;

/* Minimal COFF symbol representation with only the used field */
struct coff_symbol {
  int n_sclass; /* storage class */
};

/* Constants used in the condition */
#ifndef SYMNMLEN
#define SYMNMLEN 8
#endif

#ifndef C_NT_WEAK
#define C_NT_WEAK 105 /* Value doesn't matter for triggering the bug */
#endif

#ifndef COFF_SYMBOL_UNDEFINED
#define COFF_SYMBOL_UNDEFINED 0
#endif

/* Linker info and callbacks stubs */
struct bfd_link_callbacks {
  void (*reloc_overflow)(void *info,
                         void *hroot,
                         const char *name,
                         const char *howto_name,
                         bfd_vma addend,
                         bfd *input_bfd,
                         asection *input_section,
                         bfd_vma offset);
};

struct bfd_link_info {
  struct bfd_link_callbacks *callbacks;
  void *base_file; /* Unused here */
};

/* Internal relocation entry stub */
struct internal_reloc {
  bfd_vma r_vaddr;
};

/* ------- Stubs for functions referenced by the vulnerable code ------- */
static int bfd_coff_classify_symbol(bfd *output_bfd, struct coff_symbol *sym) {
  /* Not reached if we crash first; return something deterministic */
  (void)output_bfd; (void)sym;
  return COFF_SYMBOL_UNDEFINED;
}

static const char *_bfd_coff_internal_syment_name(bfd *input_bfd,
                                                  struct coff_symbol *sym,
                                                  char *buf) {
  (void)input_bfd; (void)sym; (void)buf;
  return "dummy";
}

static void dummy_reloc_overflow_cb(void *info, void *hroot, const char *name,
                                    const char *howto_name, bfd_vma addend,
                                    bfd *input_bfd, asection *input_section,
                                    bfd_vma offset) {
  (void)info; (void)hroot; (void)name; (void)howto_name;
  (void)addend; (void)input_bfd; (void)input_section; (void)offset;
}

/* -------------------------------------------------------------------- */
/* This function contains the vulnerable pattern from coff_relocate_section */
static bool coff_relocate_section_reproducer(bfd *output_bfd,
                                             struct bfd_link_info *info,
                                             bfd *input_bfd,
                                             asection *input_section,
                                             unsigned char *contents,
                                             struct internal_reloc *rel,
                                             reloc_howto_type *howto,
                                             long val,
                                             long addend,
                                             struct coff_symbol *sym,
                                             void *h,
                                             int symndx) {
  (void)info; (void)input_bfd; (void)contents; (void)h; (void)symndx;

  /* Force the relocation status to overflow to reach the buggy branch. */
  bfd_reloc_status_type rstat = bfd_reloc_overflow;

  switch (rstat) {
    default:
      abort();
    case bfd_reloc_ok:
      break;
    case bfd_reloc_outofrange:
      fprintf(stderr, "out of range relocs not handled in reproducer\n");
      return false;
    case bfd_reloc_overflow: {
      /* This replicates the vulnerable condition:
       * If val == 0 and (addend + 4) == 0, the code unconditionally
       * dereferences sym->n_sclass without checking sym for NULL.
       * Passing sym == NULL here will crash. */
      if (val == 0
          && (addend + 4) == 0
          /* Vulnerable deref: sym is NULL in our crafted input */
          && sym->n_sclass == C_NT_WEAK
          && bfd_coff_classify_symbol(output_bfd, sym) == COFF_SYMBOL_UNDEFINED)
      {
        break;
      }

      const char *name;
      char buf[SYMNMLEN + 1];

      if (symndx == -1)
        name = "*ABS*";
      else if (h != NULL)
        name = NULL;
      else {
        name = _bfd_coff_internal_syment_name(input_bfd, sym, buf);
        if (name == NULL)
          return false;
      }

      if (info && info->callbacks && info->callbacks->reloc_overflow) {
        info->callbacks->reloc_overflow(info, (h ? h : NULL), name,
                                        howto ? howto->name : "HOWTO",
                                        (bfd_vma)0, input_bfd, input_section,
                                        rel->r_vaddr - input_section->vma);
      }
    }
  }

  return true;
}

int main(void) {
  /* Set up minimal structures to reach the vulnerable branch. */
  bfd output_bfd_obj = {0};
  bfd input_bfd_obj = {0};

  asection in_sec = {0};
  in_sec.vma = 0;
  in_sec.output_section = &in_sec;

  struct internal_reloc reloc = {0};
  reloc.r_vaddr = 0;

  reloc_howto_type howto = { .name = "TEST_RELOC" };

  struct bfd_link_callbacks cbs = { .reloc_overflow = dummy_reloc_overflow_cb };
  struct bfd_link_info info = { .callbacks = &cbs, .base_file = NULL };

  unsigned char contents[1] = {0};

  /* Craft inputs to trigger the vulnerable dereference:
   * - rstat forced to bfd_reloc_overflow inside function
   * - val == 0 and addend == -4 so that (addend + 4) == 0
   * - sym == NULL, causing sym->n_sclass to dereference a NULL pointer */
  long val = 0;
  long addend = -4;
  struct coff_symbol *sym = NULL; /* NULL to trigger the crash */

  /* Call the function containing the vulnerable logic. */
  (void)coff_relocate_section_reproducer(&output_bfd_obj, &info, &input_bfd_obj,
                                         &in_sec, contents, &reloc, &howto,
                                         val, addend, sym, NULL, 0);

  /* If we got here without crashing, the reproduction failed. */
  fprintf(stderr, "Reproducer did not trigger the bug as expected.\n");
  return 0;
}
