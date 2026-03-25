#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal typedefs to mimic BFD types. */
typedef uint64_t bfd_vma;
typedef struct bfd { int dummy; } bfd;

typedef struct asection {
  bfd_vma vma;
  size_t size;              /* Size of section in bytes. */
  unsigned char *contents;  /* Backing buffer. */
} asection;

/* Minimal relocation howto structure. */
typedef struct reloc_howto_type {
  uint64_t dst_mask;
  uint64_t src_mask;
  int size; /* 2, 4 or 8 bytes. */
} reloc_howto_type;

static int bfd_get_reloc_size(const reloc_howto_type *howto) {
  return howto->size;
}

/* Endian-agnostic minimal bfd_put_* implementations that write to memory. */
static void bfd_put_16(bfd *abfd, uint64_t val, void *location) {
  (void)abfd;
  uint16_t v = (uint16_t)val;
  unsigned char *p = (unsigned char *)location;
  /* Write 2 bytes at LOCATION. */
  p[0] = (unsigned char)(v & 0xFF);
  p[1] = (unsigned char)((v >> 8) & 0xFF);
}

static void bfd_put_32(bfd *abfd, uint64_t val, void *location) {
  (void)abfd;
  uint32_t v = (uint32_t)val;
  unsigned char *p = (unsigned char *)location;
  p[0] = (unsigned char)(v & 0xFF);
  p[1] = (unsigned char)((v >> 8) & 0xFF);
  p[2] = (unsigned char)((v >> 16) & 0xFF);
  p[3] = (unsigned char)((v >> 24) & 0xFF);
}

static void bfd_put_64(bfd *abfd, uint64_t val, void *location) {
  (void)abfd;
  uint64_t v = val;
  unsigned char *p = (unsigned char *)location;
  p[0] = (unsigned char)(v & 0xFF);
  p[1] = (unsigned char)((v >> 8) & 0xFF);
  p[2] = (unsigned char)((v >> 16) & 0xFF);
  p[3] = (unsigned char)((v >> 24) & 0xFF);
  p[4] = (unsigned char)((v >> 32) & 0xFF);
  p[5] = (unsigned char)((v >> 40) & 0xFF);
  p[6] = (unsigned char)((v >> 48) & 0xFF);
  p[7] = (unsigned char)((v >> 56) & 0xFF);
}

/*
 * A minimal reproduction of the vulnerable logic from
 * xcoff64_ppc_relocate_section in bfd/coff64-rs6000.c.
 *
 * The bug occurs when address == input_section->size is allowed,
 * then the code computes location = contents + address and writes back
 * 2/4/8 bytes to that location, overflowing past the end of the buffer.
 */
static int xcoff64_ppc_relocate_section(bfd *input_bfd,
                                        asection *input_section,
                                        unsigned char *contents,
                                        size_t address) {
  /* Permit address equal to section size (off-by-one policy). */
  if (address > input_section->size) {
    return 0; /* Out of bounds rejected, equal is allowed. */
  }

  /* Compute writeback location (just past end when address == size). */
  unsigned char *location = contents + address;

  /* Construct a howto that makes bfd_get_reloc_size return 2. */
  reloc_howto_type howto;
  howto.size = 2;            /* Choose 2-byte relocation to match the PoC. */
  howto.src_mask = 0xFFFFu;  /* Example masks. */
  howto.dst_mask = 0xFFFFu;

  /* Some dummy values to exercise the combine-then-writeback path. */
  bfd_vma relocation = 1;
  uint64_t value_to_relocate = 0x00ABu; /* 16-bit value is fine here. */

  /* Combine relocation value according to masks. */
  value_to_relocate = ((value_to_relocate & ~howto.dst_mask)
                      | (((value_to_relocate & howto.src_mask) + relocation)
                         & howto.dst_mask));

  /* Vulnerable writeback: writes 2 bytes at contents + address. */
  switch (bfd_get_reloc_size(&howto)) {
    case 2:
      bfd_put_16(input_bfd, value_to_relocate, location);
      break;
    case 4:
      bfd_put_32(input_bfd, value_to_relocate, location);
      break;
    default:
      bfd_put_64(input_bfd, value_to_relocate, location);
      break;
  }

  return 1;
}

int main(void) {
  /* Set up an input section with a small heap buffer. */
  const size_t sec_size = 16; /* Any small size works. */
  unsigned char *contents = (unsigned char *)malloc(sec_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xCC, sec_size);

  asection sec;
  sec.vma = 0;
  sec.size = sec_size;
  sec.contents = contents;

  bfd dummy_bfd; /* Not used by our stubs. */

  /* Trigger: address equal to input_section->size (off-by-one). */
  size_t address = sec.size; /* == sec_size, points just past the end. */

  /* This call will perform a 2-byte write at contents + sec.size, which is
     2 bytes past the end of the allocated buffer, triggering ASan. */
  (void)xcoff64_ppc_relocate_section(&dummy_bfd, &sec, sec.contents, address);

  /* Clean up (won't be reached if ASan aborts, but fine). */
  free(contents);
  return 0;
}
