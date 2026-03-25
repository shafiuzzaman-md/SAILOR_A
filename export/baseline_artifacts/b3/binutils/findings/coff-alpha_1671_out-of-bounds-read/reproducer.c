#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD types used in the vulnerable code path. */

typedef unsigned long bfd_vma;

struct asection;
typedef struct asection asection;

struct asection {
  bfd_vma vma;
  bfd_vma output_offset;
  asection *output_section;
};

/* Simulated global mapping like the real symndx_to_section[] used by
   coff-alpha.c:alpha_relocate_section. */
static asection **symndx_to_section = NULL;
static size_t symndx_to_section_count = 0;

/* Some of the ALPHA_R_OP_* relocation opcodes that reach the buggy path. */
enum {
  ALPHA_R_OP_PUSH = 1,
  ALPHA_R_OP_PSUB = 2,
  ALPHA_R_OP_PRSHIFT = 3
};

/* This is a minimal reproduction of the vulnerable path inside
   alpha_relocate_section. It intentionally indexes symndx_to_section
   by r_symndx without any bounds check when r_extern == 0. */
static void alpha_relocate_section_repro(int reloc_op, int r_extern, size_t r_symndx) {
  switch (reloc_op) {
    case ALPHA_R_OP_PUSH:
    case ALPHA_R_OP_PSUB:
    case ALPHA_R_OP_PRSHIFT:
      /* In the non-extern case, the real code does:
           s = symndx_to_section[r_symndx];
         without checking that r_symndx is a valid index. */
      if (!r_extern) {
        asection *s;
        /* Out-of-bounds read happens here when r_symndx >= symndx_to_section_count. */
        s = symndx_to_section[r_symndx];
        /* Use the value so the compiler keeps the read; ASan will instrument this load. */
        volatile uintptr_t sink = (uintptr_t)s;
        (void)sink;

        /* The real code would then check s == NULL and potentially use s->fields,
           but we don't need to dereference s to demonstrate the OOB read — the
           out-of-bounds array load above is sufficient for ASan to report. */
      }
      break;
    default:
      break;
  }
}

int main(void) {
  /* Allocate a very small table to make going out-of-bounds easy and obvious. */
  symndx_to_section_count = 4;
  symndx_to_section = (asection **)malloc(symndx_to_section_count * sizeof(*symndx_to_section));
  if (!symndx_to_section) {
    fprintf(stderr, "alloc failed\n");
    return 1;
  }
  memset(symndx_to_section, 0, symndx_to_section_count * sizeof(*symndx_to_section));

  /* Craft r_symndx to be well beyond the allocated array bounds. */
  size_t r_symndx = symndx_to_section_count + 1000000; /* Far OOB to ensure ASan triggers. */
  int r_extern = 0; /* Non-extern path to reach the buggy index. */

  /* Trigger the vulnerable code path corresponding to ALPHA_R_OP_* ops. */
  alpha_relocate_section_repro(ALPHA_R_OP_PUSH, r_extern, r_symndx);

  /* Cleanup (unreached if ASan aborts on error, but harmless). */
  free(symndx_to_section);
  return 0;
}
