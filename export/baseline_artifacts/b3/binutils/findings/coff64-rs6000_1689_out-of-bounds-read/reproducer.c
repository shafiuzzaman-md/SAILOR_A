#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Minimal stand-ins for BFD types used by the vulnerable code. */
typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection {
  uint64_t size;           /* Section size in bytes */
  uint64_t vma;            /* Base VMA */
  struct asection *output_section;
  uint64_t output_offset;
} asection;

typedef struct my_reloc {
  uint64_t r_vaddr;  /* Virtual address of relocation */
  uint32_t r_type;   /* Relocation type (index into function table below) */
} my_reloc;

/* howto structure and helpers mimicking BFD */
typedef struct reloc_howto_struct {
  int size;                   /* Size of the relocation field in bytes */
  int complain_on_overflow;   /* Not used here */
} reloc_howto_struct;

static inline int bfd_get_reloc_size(const reloc_howto_struct *howto) {
  return howto->size;
}

/* Endian-agnostic minimal readers to simulate bfd_get_16/32/64. */
static uint16_t bfd_get_16(bfd *abfd, const void *ptr) {
  const unsigned char *p = (const unsigned char *)ptr;
  /* Intentional 2-byte read; when ptr points at end-of-buffer this is OOB. */
  return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t bfd_get_32(bfd *abfd, const void *ptr) {
  const unsigned char *p = (const unsigned char *)ptr;
  return (uint32_t)(p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static uint64_t bfd_get_64(bfd *abfd, const void *ptr) {
  const unsigned char *p = (const unsigned char *)ptr;
  return (uint64_t)p[0]
       | ((uint64_t)p[1] << 8)
       | ((uint64_t)p[2] << 16)
       | ((uint64_t)p[3] << 24)
       | ((uint64_t)p[4] << 32)
       | ((uint64_t)p[5] << 40)
       | ((uint64_t)p[6] << 48)
       | ((uint64_t)p[7] << 56);
}

/* Prototype for the calculate_relocation callback type used in the vulnerable path. */
typedef int (*calc_func_t)(bfd *input_bfd,
                           asection *input_section,
                           bfd *output_bfd,
                           const my_reloc *rel,
                           void *sym,
                           reloc_howto_struct *howto,
                           uint64_t val,
                           int64_t addend,
                           uint64_t *relocation,
                           unsigned char *contents,
                           void *info);

/* Minimal function table and implementation: we only need index 0. */
#define XCOFF_MAX_CALCULATE_RELOCATION 1
static int calc_ok(bfd *input_bfd, asection *input_section, bfd *output_bfd,
                   const my_reloc *rel, void *sym, reloc_howto_struct *howto,
                   uint64_t val, int64_t addend, uint64_t *relocation,
                   unsigned char *contents, void *info) {
  (void)input_bfd; (void)input_section; (void)output_bfd; (void)rel; (void)sym;
  (void)val; (void)addend; (void)relocation; (void)contents; (void)info;
  /* Force a 2-byte relocation read, matching the vulnerable switch-case path. */
  howto->size = 2;
  return 1; /* success */
}

static calc_func_t xcoff64_calculate_relocation[XCOFF_MAX_CALCULATE_RELOCATION] = {
  calc_ok
};

/* Reimplementation of the vulnerable slice of xcoff64_ppc_relocate_section. */
static bool xcoff64_ppc_relocate_section(bfd *output_bfd, void *info,
                                         bfd *input_bfd, asection *input_section,
                                         unsigned char *contents, const my_reloc *rel) {
  (void)output_bfd; (void)info;
  reloc_howto_struct howto;
  uint64_t relocation = 0; /* unused but passed to calc */

  if (rel->r_type >= XCOFF_MAX_CALCULATE_RELOCATION ||
      !((*xcoff64_calculate_relocation[rel->r_type])(
          input_bfd, input_section, output_bfd, rel, NULL, &howto,
          0 /* val */, 0 /* addend */, &relocation, contents, info)))
    return false;

  /* address */
  uint64_t address = rel->r_vaddr - input_section->vma;
  unsigned char *location = contents + address;

  /* Vulnerable bounds check: allows address == size. */
  if (address > input_section->size)
    abort();

  uint64_t value_to_relocate;
  switch (bfd_get_reloc_size(&howto)) {
    case 2:
      value_to_relocate = bfd_get_16(input_bfd, location);
      break;
    case 4:
      value_to_relocate = bfd_get_32(input_bfd, location);
      break;
    default:
      value_to_relocate = bfd_get_64(input_bfd, location);
      break;
  }

  /* Use the value to prevent optimization away. */
  printf("Relocated value: %llu\n", (unsigned long long)value_to_relocate);
  return true;
}

int main(void) {
  /* Create a small contents buffer. */
  const size_t sz = 16;
  unsigned char *contents = (unsigned char *)malloc(sz);
  if (!contents) return 1;
  memset(contents, 0x41, sz); /* 'A' */

  /* Define an input section whose size equals the contents size, with some VMA. */
  asection sec;
  memset(&sec, 0, sizeof(sec));
  sec.size = sz;
  sec.vma = 0x1000;

  /* Craft a reloc whose r_vaddr maps to address == input_section->size. */
  my_reloc rel;
  rel.r_type = 0; /* Index into our calc function table. */
  rel.r_vaddr = sec.vma + sec.size; /* address == size -> passes faulty check */

  /* Call the vulnerable function: it will read 2 bytes at contents + size. */
  (void)xcoff64_ppc_relocate_section(NULL, NULL, NULL, &sec, contents, &rel);

  free(contents);
  return 0;
}
