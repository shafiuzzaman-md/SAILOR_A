#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif

/* Minimal stubs of BFD-like structures and constants to reproduce the crash
   in coff_gc_sweep_symbol from bfd/coffgen.c. */

typedef struct bfd bfd;

typedef struct asection {
  unsigned int flags;
  unsigned char gc_mark;
  struct asection *next;
  bfd *owner;           /* Critical: will be NULL for ABS section to trigger bug */
  const char *name;
} asection;

struct bfd {
  unsigned int flags;
  struct { bfd *next; } link;
};

/* Constants/magic values (only for compilation; values themselves are irrelevant). */
#define DYNAMIC (1u << 5)
#define C_HIDDEN 0x2a

/* Hash entry types, mimicking bfd_link_hash_* */
enum bfd_link_hash_type {
  bfd_link_hash_undefined = 0,
  bfd_link_hash_defined   = 1,
  bfd_link_hash_defweak   = 2,
  bfd_link_hash_warning   = 3
};

/* Minimal root hash node with the exact field layout used. */
struct bfd_link_hash_common {
  enum bfd_link_hash_type type;
  union {
    struct { asection *section; } def;   /* used when type == defined/defweak */
    struct { void *link; } i;            /* used when type == warning */
  } u;
};

struct coff_link_hash_entry {
  struct bfd_link_hash_common root;
  int symbol_class;
};

/* Undeclared external from real BFD; we provide a stub. */
static asection und_section = {0};
static asection *bfd_und_section_ptr = &und_section;

/* Vulnerable function (reduced to the relevant logic). */
static bool
coff_gc_sweep_symbol(struct coff_link_hash_entry *h, void *data ATTRIBUTE_UNUSED)
{
  if (h->root.type == bfd_link_hash_warning)
    h = (struct coff_link_hash_entry *) h->root.u.i.link;

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && !h->root.u.def.section->gc_mark
      && !(h->root.u.def.section->owner->flags & DYNAMIC))  /* NULL deref here */
    {
      /* Do our best to hide the symbol. */
      h->root.u.def.section = bfd_und_section_ptr;
      h->symbol_class = C_HIDDEN;
    }

  return true;
}

int main(void)
{
  /* Create an ABS (absolute) section with owner == NULL (as in BFD for ABS). */
  asection abs_sec;
  memset(&abs_sec, 0, sizeof(abs_sec));
  abs_sec.name = "ABS";
  abs_sec.gc_mark = 0;   /* Ensure the !gc_mark condition holds. */
  abs_sec.owner = NULL;  /* Critical to trigger NULL deref of owner->flags. */

  /* Create a defined symbol whose section is the ABS section. */
  struct coff_link_hash_entry h;
  memset(&h, 0, sizeof(h));
  h.root.type = bfd_link_hash_defined;        /* defined symbol */
  h.root.u.def.section = &abs_sec;            /* points to ABS with NULL owner */

  fprintf(stderr, "About to call coff_gc_sweep_symbol... this should crash due to NULL owner->flags access.\n");

  /* This call triggers the NULL pointer dereference inside the condition. */
  (void) coff_gc_sweep_symbol(&h, NULL);

  /* We should never reach here. */
  fprintf(stderr, "If you see this, the crash did not occur as expected.\n");
  return 0;
}
