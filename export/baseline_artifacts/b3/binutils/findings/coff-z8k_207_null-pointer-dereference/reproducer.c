#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

/* Minimal re-declarations of BFD-like types used by coff-z8k.c */
typedef unsigned char bfd_byte;
typedef unsigned long bfd_size_type;
typedef uint64_t bfd_vma;

typedef struct bfd { int dummy; } bfd;

typedef struct asection {
  bfd_size_type vma;
  unsigned int flags;
} asection;

typedef struct asymbol {
  asection *section;
} asymbol;

struct reloc_howto_type {
  unsigned int type;
  bfd_size_type size; /* For bfd_get_reloc_size */
};

typedef struct arelent {
  bfd_vma address;
  struct reloc_howto_type *howto; /* This will be NULL to trigger the bug */
  asymbol **sym_ptr_ptr;
  bfd_vma addend;
} arelent;

/* Linker callback plumbing used by extra_case error path. */
struct bfd_link_callbacks {
  void (*einfo) (const char *fmt, ...);
};

struct bfd_link_info {
  struct bfd_link_callbacks *callbacks;
};

struct bfd_link_order {
  union {
    struct {
      asection *section;
    } indirect;
  } u;
};

/* Relocation type constants referenced by the switch. */
enum {
  R_IMM8  = 1,
  R_IMM32 = 2
};

/* Stub implementations of BFD helper functions referenced by extra_case. */
static bfd_size_type bfd_get_section_limit_octets(bfd *abfd, asection *sec) {
  (void)abfd; (void)sec;
  return 64; /* Arbitrary non-zero size */
}

/* This intentionally dereferences the howto pointer, matching real BFD behavior.
   Passing NULL here will cause a NULL-pointer dereference, reproducing the bug
   at the same program point as in the original code (line 207). */
static bfd_size_type bfd_get_reloc_size(const struct reloc_howto_type *howto) {
  return howto->size; /* Will crash if howto == NULL */
}

/* Stubs for functions that won't be reached due to the crash but needed to link. */
static bfd_vma bfd_coff_reloc16_get_value(arelent *reloc, struct bfd_link_info *info, asection *input_section) {
  (void)reloc; (void)info; (void)input_section; return 0; }
static void bfd_put_8(bfd *abfd, bfd_vma val, bfd_byte *dst) { (void)abfd; *dst = (bfd_byte)val; }
static void bfd_put_32(bfd *abfd, bfd_vma val, bfd_byte *dst) {
  (void)abfd;
  dst[0] = (bfd_byte)((val >> 24) & 0xff);
  dst[1] = (bfd_byte)((val >> 16) & 0xff);
  dst[2] = (bfd_byte)((val >> 8) & 0xff);
  dst[3] = (bfd_byte)(val & 0xff);
}

/* Simple printf-like callback for error reporting. */
static void einfo_stub(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
}

/* Reimplementation of the vulnerable function from bfd/coff-z8k.c. */
static bool extra_case(bfd *in_abfd,
                       struct bfd_link_info *link_info,
                       struct bfd_link_order *link_order,
                       arelent *reloc,
                       bfd_byte *data,
                       size_t *src_ptr,
                       size_t *dst_ptr)
{
  asection *input_section = link_order->u.indirect.section;
  bfd_size_type end = bfd_get_section_limit_octets(in_abfd, input_section);
  /* Vulnerable dereference via NULL howto passed to bfd_get_reloc_size. */
  bfd_size_type reloc_size = bfd_get_reloc_size(reloc->howto);

  if (*src_ptr > end || reloc_size > end - *src_ptr) {
    link_info->callbacks->einfo("%s", "relocation goes out of range\n");
    return false;
  }

  switch (reloc->howto->type) {
    case R_IMM8:
      bfd_put_8(in_abfd, bfd_coff_reloc16_get_value(reloc, link_info, input_section), data + *dst_ptr);
      *dst_ptr += 1;
      *src_ptr += 1;
      break;
    case R_IMM32:
      if (!(*reloc->sym_ptr_ptr)->section->flags) {
        bfd_put_32(in_abfd, bfd_coff_reloc16_get_value(reloc, link_info, input_section), data + *dst_ptr);
      } else {
        bfd_vma dst = bfd_coff_reloc16_get_value(reloc, link_info, input_section);
        dst = (dst & 0xffff) | ((dst & 0xff0000) << 8) | 0x80000000;
        bfd_put_32(in_abfd, dst, data + *dst_ptr);
      }
      *dst_ptr += 4;
      *src_ptr += 4;
      break;
    default:
      break;
  }

  return true;
}

int main(void) {
  /* Set up minimal structures to reach extra_case. */
  bfd in_abfd = {0};
  asection sec = {0};
  struct bfd_link_order lorder; memset(&lorder, 0, sizeof(lorder));
  lorder.u.indirect.section = &sec;

  struct bfd_link_callbacks callbacks = { .einfo = einfo_stub };
  struct bfd_link_info linfo = { .callbacks = &callbacks };

  /* Craft a reloc entry with an unsupported/unknown type, modeled here by
     setting howto = NULL, as would happen after rtype2howto() fails. */
  arelent reloc; memset(&reloc, 0, sizeof(reloc));
  reloc.howto = NULL; /* This triggers the NULL dereference inside extra_case. */

  /* Dummy symbol pointer to satisfy structure layout if ever used. */
  asymbol sym = { .section = &sec };
  asymbol *symptr = &sym;
  reloc.sym_ptr_ptr = &symptr;

  bfd_byte data[16]; memset(data, 0, sizeof(data));
  size_t src = 0, dst = 0;

  /* This call should crash with a NULL-pointer dereference at
     bfd_get_reloc_size(reloc->howto). */
  (void)extra_case(&in_abfd, &linfo, &lorder, &reloc, data, &src, &dst);

  /* If the bug does not trigger, indicate unexpected behavior. */
  fprintf(stderr, "Unexpectedly survived extra_case() without crash.\n");
  return 0;
}
