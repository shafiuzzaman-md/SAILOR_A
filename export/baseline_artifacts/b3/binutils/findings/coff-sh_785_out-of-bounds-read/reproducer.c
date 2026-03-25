#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

/* Minimal stand-ins for BFD types/API used by the vulnerable code */
typedef struct bfd { int dummy; } bfd;
typedef uint64_t bfd_vma;
typedef int64_t  bfd_signed_vma;
typedef unsigned char bfd_byte;

/* Section structure with just the fields we need */
typedef struct asection {
  bfd_vma vma;
  bfd_vma size;
  bfd_byte *contents;
} asection;

/* Minimal internal reloc structure */
struct internal_reloc {
  bfd_vma r_vaddr;
  uint32_t r_offset; /* 32-bit field in real code; sign-extended by logic */
  int r_type;
};

/* Reloc type constants we need */
#define R_SH_CODE 1
#define R_SH_USES 2

/* Stub error handler to satisfy the calls in the code path */
static void _bfd_error_handler(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fputc('\n', stderr);
  va_end(ap);
}

/* bfd_get_16 reads a 16-bit value from the buffer; endian doesn't matter for OOB */
static unsigned short bfd_get_16(bfd *abfd, const void *ptr) {
  (void)abfd;
  const unsigned unsigned_char *p = (const unsigned unsigned_char *)ptr;
  /* Perform two separate byte reads so ASan can catch the out-of-bounds b[1] */
  unsigned int b0 = p[0];
  unsigned int b1 = p[1]; /* If p points to the last valid byte, this is OOB */
  /* Use little-endian composition (arbitrary for this reproducer) */
  return (unsigned short)((b1 << 8) | b0);
}

/* Simplified version of the vulnerable sh_relax_section code path */
static void sh_relax_section(bfd *abfd,
                             asection *sec,
                             struct internal_reloc *internal_relocs,
                             struct internal_reloc *irelend) {
  bool have_code = false;
  bfd_byte *contents = NULL;

  for (struct internal_reloc *irel = internal_relocs; irel < irelend; ++irel) {
    bfd_vma laddr, paddr, symval;
    unsigned short insn;
    (void)paddr; (void)symval; /* Unused in this minimal reproducer */

    if (irel->r_type == R_SH_CODE)
      have_code = true;

    if (irel->r_type != R_SH_USES)
      continue;

    /* Get the section contents. In the real code this may allocate/fetch; here we just use sec->contents. */
    if (contents == NULL) {
      contents = sec->contents;
    }

    /* Compute the load address as in the original code. */
    laddr = irel->r_vaddr - sec->vma + 4;
    /* Careful to sign extend the 32-bit offset. */
    laddr += ((uint64_t)(irel->r_offset & 0xffffffffu) ^ 0x80000000ull) - 0x80000000ll;

    if (laddr >= sec->size) {
      _bfd_error_handler("%s: %#" PRIx64 ": warning: bad R_SH_USES offset", "repro.o", (uint64_t) irel->r_vaddr);
      continue;
    }

    /* Vulnerable read: if laddr == sec->size - 1, this reads 1 byte past the end. */
    insn = bfd_get_16(abfd, contents + laddr);

    /* Prevent optimizing away insn. */
    if ((insn & 0xffffu) == 0xdead)
      fprintf(stderr, "Impossible value to avoid DCE: %u\n", insn);

    /* We've triggered the read; stop. */
    break;
  }

  (void)have_code; /* Silence unused warning */
}

int main(void) {
  /* Create a section buffer whose last byte will be used as the start of a 16-bit read */
  const size_t sz = 8; /* Any size >= 5 works; we choose 8 for clarity */
  bfd_byte *buf = (bfd_byte *)malloc(sz);
  if (!buf) {
    perror("malloc");
    return 1;
  }
  for (size_t i = 0; i < sz; ++i) buf[i] = (bfd_byte)(i & 0xff);

  asection sec = {0};
  sec.vma = 0;         /* Simplify address math */
  sec.size = (bfd_vma)sz;
  sec.contents = buf;

  /* Craft a relocation such that laddr == sec->size - 1
     laddr = r_vaddr - vma + 4 + signext32(r_offset)
     Choose r_vaddr = 0, vma = 0, so laddr = 4 + signext32(r_offset)
     Set signext32(r_offset) = sz - 5 => r_offset = sz - 5 (fits positive range) */
  struct internal_reloc rel;
  rel.r_type = R_SH_USES;
  rel.r_vaddr = 0;            /* so base is 4 */
  rel.r_offset = (uint32_t)(sz - 5); /* so laddr = 4 + (sz - 5) = sz - 1 */

  bfd dummy_bfd = {0};

  /* This call triggers the out-of-bounds 1-byte read in bfd_get_16 */
  sh_relax_section(&dummy_bfd, &sec, &rel, &rel + 1);

  free(buf);
  return 0;
}
