// Standalone reproducer for the OOB read in coff_relocate_section (bfd/cofflink.c:3072)
// Compile with: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal re-declarations to model the vulnerable path

#define C_NT_WEAK 0x70

typedef enum {
  bfd_link_hash_undefined = 0,
  bfd_link_hash_defined   = 1,
  bfd_link_hash_defweak   = 2,
  bfd_link_hash_undefweak = 3
} bfd_link_hash_type;

struct bfd_section { int dummy; };

struct coff_link_hash_entry; // fwd decl

struct coff_obj_tdata {
  struct coff_link_hash_entry **sym_hashes;
  size_t sym_hashes_count; // for info/debug only; no bounds checks in vulnerable path
};

struct bfd_tdata {
  struct coff_obj_tdata *coff_obj_data;
};

struct bfd {
  struct bfd_tdata tdata;
};

struct x_sym_tagndx { uint32_t u32; };
struct x_sym { struct x_sym_tagndx x_tagndx; };

struct auxent_mock { struct x_sym x_sym; };

struct coff_link_hash_entry_root_def {
  struct bfd_section *section;
  unsigned long value;
};

struct coff_link_hash_entry_root {
  bfd_link_hash_type type;
  union { struct coff_link_hash_entry_root_def def; } u;
};

struct coff_link_hash_entry {
  struct coff_link_hash_entry_root root;
  int symbol_class;
  int numaux;
  struct auxent_mock *aux;
  struct bfd *auxbfd;
};

// Stub of the vulnerable function path. It intentionally lacks bounds checks
// on x_tagndx when indexing sym_hashes to reproduce the OOB read.
static void coff_relocate_section(struct coff_link_hash_entry *h) {
  if (h->root.type == bfd_link_hash_undefweak) {
    if (h->symbol_class == C_NT_WEAK && h->numaux == 1) {
      // Vulnerable access: uses x_tagndx.u32 as an array index with no validation
      struct coff_link_hash_entry *h2 =
        h->auxbfd->tdata.coff_obj_data->sym_hashes[h->aux->x_sym.x_tagndx.u32];
      // Use h2 in a benign way to avoid unused warnings (do not dereference)
      printf("h2=%p\n", (void *)h2);
    } else {
      printf("Not the weak+aux path.\n");
    }
  } else {
    printf("Not undefweak type.\n");
  }
}

int main(void) {
  // Allocate a very small sym_hashes array
  size_t sym_count = 4;
  struct coff_obj_tdata *coff = (struct coff_obj_tdata *)calloc(1, sizeof(*coff));
  if (!coff) return 1;
  coff->sym_hashes = (struct coff_link_hash_entry **)calloc(sym_count, sizeof(struct coff_link_hash_entry *));
  if (!coff->sym_hashes) return 1;
  coff->sym_hashes_count = sym_count;

  struct bfd *abfd = (struct bfd *)calloc(1, sizeof(*abfd));
  if (!abfd) return 1;
  abfd->tdata.coff_obj_data = coff;

  // Create an AUX record with an out-of-range x_tagndx
  struct auxent_mock *aux = (struct auxent_mock *)calloc(1, sizeof(*aux));
  if (!aux) return 1;
  aux->x_sym.x_tagndx.u32 = 1000; // Deliberately out-of-bounds index

  // Build a C_NT_WEAK undefweak symbol with one aux record
  struct coff_link_hash_entry h;
  memset(&h, 0, sizeof(h));
  h.root.type = bfd_link_hash_undefweak;
  h.symbol_class = C_NT_WEAK;
  h.numaux = 1;
  h.aux = aux;
  h.auxbfd = abfd;

  printf("About to trigger OOB read: index=%u, sym_hashes_count=%zu\n",
         aux->x_sym.x_tagndx.u32, coff->sym_hashes_count);

  // Triggers the out-of-bounds read
  coff_relocate_section(&h);

  // Cleanup (may not be reached if ASan aborts on OOB)
  free(coff->sym_hashes);
  free(coff);
  free(aux);
  free(abfd);
  return 0;
}
