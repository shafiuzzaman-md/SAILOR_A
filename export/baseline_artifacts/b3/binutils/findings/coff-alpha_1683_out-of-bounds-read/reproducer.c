#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal re-declarations to mirror the vulnerable path in
   bfd/coff-alpha.c:alpha_relocate_section */

enum {
  ALPHA_R_OP_PUSH = 1,
  ALPHA_R_OP_PSUB = 2,
  ALPHA_R_OP_PRSHIFT = 3
};

/* Dummy stand-in for the real structure. */
struct ecoff_link_hash_entry {
  int dummy;
};

/* This function intentionally mirrors the vulnerable indexing logic from
   alpha_relocate_section for the ALPHA_R_OP_* cases when r_extern is set. */
static void alpha_relocate_section(struct ecoff_link_hash_entry **sym_hashes,
                                   size_t sym_hashes_count,
                                   unsigned int r_symndx,
                                   int r_extern,
                                   int r_type)
{
  /* Only handle the ALPHA_R_OP_* relocation operators, like the original code. */
  switch (r_type) {
    case ALPHA_R_OP_PUSH:
    case ALPHA_R_OP_PSUB:
    case ALPHA_R_OP_PRSHIFT:
      if (!r_extern) {
        /* In the non-extern case the original code uses symndx_to_section, which
           is safe in our reproducer. We just return early here. */
        return;
      } else {
        /* Vulnerable path: uses r_symndx as an index into sym_hashes without any
           bounds checking. This reproduces the out-of-bounds read. */
        fprintf(stderr, "Triggering OOB read: r_symndx=%u, sym_hashes_count=%zu\n",
                r_symndx, sym_hashes_count);

        /* This line mirrors the vulnerable access:
           h = sym_hashes[r_symndx];
           Accessing beyond the end of sym_hashes causes an ASan-detected
           heap-buffer-overflow (read). */
        struct ecoff_link_hash_entry *h = sym_hashes[r_symndx];

        /* Use the read value so the compiler cannot optimize it away. */
        if (h == NULL) {
          fprintf(stderr, "Read a NULL pointer from out-of-bounds slot.\n");
        } else {
          fprintf(stderr, "Read a non-NULL pointer from out-of-bounds slot: %p\n", (void*)h);
        }
      }
      break;
    default:
      /* Not relevant for this reproducer. */
      break;
  }
}

int main(void)
{
  /* Allocate a small sym_hashes array to make OOB reads easy to detect. */
  const size_t sym_hashes_count = 8;
  struct ecoff_link_hash_entry **sym_hashes =
      (struct ecoff_link_hash_entry**)calloc(sym_hashes_count, sizeof(*sym_hashes));
  if (!sym_hashes) {
    perror("calloc");
    return 1;
  }

  /* Craft an out-of-bounds index that exceeds the number of entries. */
  unsigned int r_symndx = (unsigned int)(sym_hashes_count + 100);

  /* Force the vulnerable branch (extern symbol) and one of the ALPHA_R_OP_* types. */
  int r_extern = 1; /* non-zero -> extern */
  int r_type = ALPHA_R_OP_PUSH; /* triggers the ALPHA_R_OP_* path */

  /* Call the stub that mirrors the vulnerable logic. Under ASan this will
     report a heap-buffer-overflow read on sym_hashes[r_symndx]. */
  alpha_relocate_section(sym_hashes, sym_hashes_count, r_symndx, r_extern, r_type);

  /* Clean up (not reached if ASan aborts on error, but harmless). */
  free(sym_hashes);
  return 0;
}
