#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type re-declarations to model the relevant BFD structures. */
typedef uint8_t bfd_byte;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection {
  bfd *owner;
  unsigned long vma;
} asection;

/* Modeled after reloc_howto_type in BFD. */
typedef struct reloc_howto_type {
  int type;
  int partial_inplace;
  unsigned long src_mask;
  const char *name;
} reloc_howto_type;

typedef struct asymbol {
  const char *name;
} asymbol;

typedef struct arelent {
  unsigned long address;
  long addend;
  reloc_howto_type *howto; /* This will be NULL to trigger the bug. */
  asymbol **sym_ptr_ptr;
} arelent;

/* Callback interface (very simplified). */
struct bfd_link_callbacks {
  void (*einfo)(const char *fmt, ...);
  void (*reloc_overflow)(void *link_info,
                         void *reloc_entry,
                         const char *sym_name,
                         const char *howto_name,
                         long addend,
                         void *owner_bfd,
                         void *section,
                         unsigned long address);
};

struct bfd_link_info {
  struct bfd_link_callbacks *callbacks;
};

struct bfd_link_order_indirect {
  asection *section;
};

struct bfd_link_order {
  union {
    struct bfd_link_order_indirect indirect;
  } u;
};

/* Fake relocation type values used in the switch. */
enum {
  R_OFF8 = 1,
  R_BYTE3 = 2,
  R_BYTE2 = 3
};

/* Stubs for BFD helper functions used by extra_case. */
static unsigned long bfd_get_section_limit_octets(bfd *abfd, asection *sec) {
  (void)abfd; (void)sec;
  return 64; /* Arbitrary small limit to pass range checks. */
}

static unsigned long bfd_get_reloc_size(const reloc_howto_type *howto) {
  /* IMPORTANT: Do not dereference howto here to mirror the bug behavior where
     extra_case first passes a NULL howto to this function. */
  (void)howto;
  return 1; /* minimal relocation size */
}

static int bfd_coff_reloc16_get_value(arelent *reloc, struct bfd_link_info *info, asection *sec) {
  (void)reloc; (void)info; (void)sec;
  return 0; /* Value is irrelevant for this reproducer. */
}

static unsigned char bfd_get_8(bfd *abfd, const void *p) {
  (void)abfd; return *(const unsigned char *)p;
}

static void bfd_put_8(bfd *abfd, unsigned char val, void *p) {
  (void)abfd; *(unsigned char *)p = val;
}

/* Simple printf-like callback implementations. */
static void cb_einfo(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
}

static void cb_reloc_overflow(void *link_info,
                              void *reloc_entry,
                              const char *sym_name,
                              const char *howto_name,
                              long addend,
                              void *owner_bfd,
                              void *section,
                              unsigned long address) {
  (void)link_info; (void)reloc_entry; (void)sym_name; (void)howto_name;
  (void)addend; (void)owner_bfd; (void)section; (void)address;
  fprintf(stderr, "reloc_overflow callback invoked\n");
}

/* Simplified reimplementation of the vulnerable function from coff-z80.c. */
static bool extra_case(bfd *in_abfd,
                       struct bfd_link_info *link_info,
                       struct bfd_link_order *link_order,
                       arelent *reloc,
                       bfd_byte *data,
                       size_t *src_ptr,
                       size_t *dst_ptr)
{
  asection *input_section = link_order->u.indirect.section;
  unsigned long end = bfd_get_section_limit_octets(in_abfd, input_section);
  unsigned long reloc_size = bfd_get_reloc_size(reloc->howto); /* reloc->howto is NULL */

  if (*src_ptr > end || reloc_size > end - *src_ptr) {
    link_info->callbacks->einfo("relocation goes out of range\n");
    return false;
  }

  int val = bfd_coff_reloc16_get_value(reloc, link_info, input_section);

  /* BUG: Unconditional dereference of reloc->howto follows. */
  switch (reloc->howto->type) { /* NULL dereference here */
    case R_OFF8:
      if (reloc->howto->partial_inplace)
        val += (signed char)(bfd_get_8(in_abfd, data + *src_ptr) & reloc->howto->src_mask);
      if (val > 127 || val < -128) {
        link_info->callbacks->reloc_overflow(link_info, NULL, "sym", "HOWTO", reloc->addend,
                                             input_section->owner, input_section, reloc->address);
        return false;
      }
      bfd_put_8(in_abfd, (unsigned char)val, data + *dst_ptr);
      *dst_ptr += 1;
      *src_ptr += 1;
      break;
    case R_BYTE3:
      bfd_put_8(in_abfd, (unsigned char)(val >> 24), data + *dst_ptr);
      *dst_ptr += 1;
      *src_ptr += 1;
      break;
    case R_BYTE2:
      bfd_put_8(in_abfd, (unsigned char)(val >> 16), data + *dst_ptr);
      *dst_ptr += 1;
      *src_ptr += 1;
      break;
    default:
      break;
  }

  return true;
}

int main(void) {
  /* Set up minimal objects needed to reach the vulnerable path. */
  bfd in_bfd = {0};
  asection sec = { .owner = &in_bfd, .vma = 0 };

  struct bfd_link_order order;
  order.u.indirect.section = &sec;

  struct bfd_link_callbacks cbs = { cb_einfo, cb_reloc_overflow };
  struct bfd_link_info info = { .callbacks = &cbs };

  /* Construct a relocation entry with howto = NULL to simulate unknown type. */
  asymbol sym = { .name = "sym" };
  asymbol *sym_ptr = &sym;

  arelent rel;
  memset(&rel, 0, sizeof(rel));
  rel.howto = NULL;           /* Critical: simulate rtype2howto() failure */
  rel.sym_ptr_ptr = &sym_ptr; /* Not used here but filled for completeness */
  rel.addend = 0;
  rel.address = 0;

  uint8_t data[64] = {0};
  size_t src = 0, dst = 0;

  /* This call should crash with ASan due to NULL dereference of reloc->howto. */
  (void)extra_case(&in_bfd, &info, &order, &rel, data, &src, &dst);

  /* If the program reaches here, something went wrong. */
  printf("Unexpectedly survived the NULL dereference.\n");
  return 0;
}
