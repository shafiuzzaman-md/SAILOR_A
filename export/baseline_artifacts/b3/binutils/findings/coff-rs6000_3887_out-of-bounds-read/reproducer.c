#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by xcoff_ppc_relocate_section. */
typedef uint64_t bfd_vma;
typedef struct bfd { int dummy; } bfd;

typedef struct asection {
  bfd_vma vma;
  size_t size;
} asection;

typedef struct internal_reloc {
  bfd_vma r_vaddr;
  unsigned int r_type;
} internal_reloc;

/* Stand-in for BFD howto. We only need the reloc size. */
typedef struct reloc_howto_struct {
  int size; /* in bytes (2 or 4) */
  int complain_on_overflow; /* unused here */
} reloc_howto_type;

/* Minimal helpers mirroring BFD accessors used at the crash site. */
static inline int bfd_get_reloc_size(const reloc_howto_type *howto) {
  return howto->size;
}

static inline uint32_t bfd_get_16(bfd *abfd, const unsigned char *p) {
  /* Endianness doesn't matter for triggering the OOB read. */
  return ((uint32_t)p[0] << 8) | (uint32_t)p[1];
}

static inline uint32_t bfd_get_32(bfd *abfd, const unsigned char *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

/* A pared-down version of the vulnerable function containing the buggy logic
   from bfd/coff-rs6000.c:xcoff_ppc_relocate_section. */
bool xcoff_ppc_relocate_section(bfd *input_bfd,
                                asection *input_section,
                                bfd *output_bfd,
                                internal_reloc *rel,
                                unsigned char *contents) {
  reloc_howto_type howto;
  /* Force a 2-byte relocation to hit bfd_get_16 path. */
  howto.size = 2;
  howto.complain_on_overflow = 0;

  /* address */
  bfd_vma address = rel->r_vaddr - input_section->vma;
  unsigned char *location = contents + address;

  if (address > input_section->size)
    abort();

  /* Get the value we are going to relocate (BUG: may read past end). */
  unsigned int value_to_relocate;
  if (2 == bfd_get_reloc_size(&howto))
    value_to_relocate = bfd_get_16(input_bfd, location);
  else
    value_to_relocate = bfd_get_32(input_bfd, location);

  /* Prevent optimizing out. */
  volatile unsigned int sink = value_to_relocate;
  (void)sink;
  return true;
}

int main(void) {
  /* Set up a tiny input section with size N. */
  const size_t N = 16; /* small buffer to clearly trigger OOB read */
  asection sec;
  sec.vma = 0x1000;
  sec.size = N;

  /* Allocate contents of exactly N bytes. ASan will place a redzone after it. */
  unsigned char *contents = (unsigned char *)malloc(N);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  /* Initialize contents with a pattern. */
  for (size_t i = 0; i < N; i++) contents[i] = (unsigned char)i;

  /* Craft a relocation whose r_vaddr lands exactly at the end of the section. */
  internal_reloc rel;
  rel.r_type = 0; /* doesn't matter for this minimal reproducer */
  rel.r_vaddr = sec.vma + sec.size; /* address == size, passes > check */

  bfd in_bfd = {0};
  bfd out_bfd = {0};

  /* This call will perform bfd_get_16 at contents + size, reading 2 bytes
     out of bounds (exactly the bug). */
  (void)xcoff_ppc_relocate_section(&in_bfd, &sec, &out_bfd, &rel, contents);

  /* If ASan didn't abort yet (it should), free and exit. */
  free(contents);
  return 0;
}
