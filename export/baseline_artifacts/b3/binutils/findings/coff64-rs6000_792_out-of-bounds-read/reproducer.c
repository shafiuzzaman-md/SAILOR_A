#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/* Self-contained minimal type/struct stubs to model the vulnerable code path. */

typedef unsigned long long bfd_vma;
typedef unsigned char bfd_byte;

/* Forward decls */
struct xcoff_link_hash_entry;

/* Minimal BFD-like structures needed by the function signature. */
typedef struct bfd {
  struct xcoff_link_hash_entry **sym_hashes; /* obj_xcoff_sym_hashes(abfd) */
  size_t sym_hashes_count;                   /* number of entries */
} bfd;

typedef struct asection {
  bfd_vma vma;
  bfd_vma size;
} asection;

/* Hash structures emulating the layout used by the real code. */
struct bfd_hash_entry { const char *string; };

enum bfd_link_hash_type {
  bfd_link_hash_defined = 0,
  bfd_link_hash_defweak = 1,
  bfd_link_hash_undefined = 2
};

struct bfd_link_hash_entry {
  struct bfd_hash_entry root;   /* root.root.string in the source */
  enum bfd_link_hash_type type; /* root.type in the source */
};

struct xcoff_link_hash_entry {
  struct bfd_link_hash_entry root; /* h->root.* */
  int smclas;                      /* h->smclas */
};

/* Other parameter stubs. */
struct internal_reloc {
  int r_symndx;     /* used as index into obj_xcoff_sym_hashes */
  unsigned long r_vaddr;
};

struct internal_syment { int dummy; };
struct reloc_howto_struct { int dummy; };
struct bfd_link_info { int dummy; };

/* Magic constants used by the original code (not needed to trigger the bug). */
#ifndef XMC_GL
#define XMC_GL 0x08
#endif

/* Accessor modeled after the real macro to fetch the symbol hash array. */
static inline struct xcoff_link_hash_entry **obj_xcoff_sym_hashes(bfd *abfd) {
  return abfd->sym_hashes;
}

/* A volatile sink to prevent the compiler from optimizing away the faulty read. */
static volatile struct xcoff_link_hash_entry *oob_sink;

/* Vulnerable function (trimmed to the part that triggers the OOB read). */
static bool xcoff64_reloc_type_br(bfd *input_bfd,
                                  asection *input_section,
                                  bfd *output_bfd,
                                  struct internal_reloc *rel,
                                  struct internal_syment *sym,
                                  struct reloc_howto_struct *howto,
                                  bfd_vma val,
                                  bfd_vma addend,
                                  bfd_vma *relocation,
                                  bfd_byte *contents,
                                  struct bfd_link_info *info) {
  (void)input_section; (void)output_bfd; (void)sym; (void)howto;
  (void)val; (void)addend; (void)relocation; (void)contents; (void)info;

  struct xcoff_link_hash_entry *h;

  /* Lower-bound check only (matches the buggy code). */
  if (0 > rel->r_symndx)
    return false;

  /* Out-of-bounds read: no upper-bound check on r_symndx before indexing. */
  /* This mirrors line 792 in the provided source context. */
  h = obj_xcoff_sym_hashes(input_bfd)[rel->r_symndx];

  /* Use the value so the load is not optimized out. */
  oob_sink = h;

  /* We don't need to proceed further to demonstrate the bug. */
  return true;
}

int main(void) {
  /* Set up a fake bfd with a very small symbol hash array. */
  bfd *abfd = (bfd *)calloc(1, sizeof(bfd));
  if (!abfd) {
    perror("calloc abfd");
    return 1;
  }

  /* Allocate a tiny array of 4 pointers to act as the symbol hash table. */
  size_t n_hashes = 4;
  abfd->sym_hashes = (struct xcoff_link_hash_entry **)calloc(n_hashes, sizeof(*abfd->sym_hashes));
  abfd->sym_hashes_count = n_hashes;
  if (!abfd->sym_hashes) {
    perror("calloc sym_hashes");
    return 1;
  }

  /* Dummy section and other parameters. Not used to hit the OOB. */
  asection sec = { .vma = 0, .size = 16 };
  struct internal_reloc rel = {0};
  struct internal_syment syment = {0};
  struct reloc_howto_struct howto = {0};
  struct bfd_link_info info = {0};
  bfd_vma relocation = 0;
  bfd_byte contents[16] = {0};

  /* Craft the relocation with an out-of-range symbol index: >= abfd->sym_hashes_count. */
  rel.r_symndx = 1000; /* Deliberately far beyond the 4-element array. */
  rel.r_vaddr = 0;

  /* Call the vulnerable function. With ASan enabled, this should report an OOB read. */
  (void)xcoff64_reloc_type_br(abfd, &sec, NULL, &rel, &syment, &howto,
                              0, 0, &relocation, contents, &info);

  /* Cleanup (not reached if ASan aborts, but harmless). */
  free(abfd->sym_hashes);
  free(abfd);

  return 0;
}
