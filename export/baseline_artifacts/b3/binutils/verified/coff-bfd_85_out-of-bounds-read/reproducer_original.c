#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal stubs and type re-declarations to model the vulnerable code path. */

typedef struct bfd {
  int dummy;
} bfd;

enum bfd_error {
  bfd_error_invalid_operation = 1
};

static void bfd_set_error(enum bfd_error e) {
  (void)e;
}

#define BFD_ASSERT(x) do { (void)(x); } while (0)

/* Forward decl for obj_raw_syments stub. */
struct combined_entry_type;
static inline struct combined_entry_type* obj_raw_syments(bfd *abfd) {
  (void)abfd;
  return NULL;
}

/* A very loose approximation of the union used in BFD. We only need it to be
   large enough so that copying it by value performs a noticeable read. */
union internal_auxent {
  struct {
    struct {
      struct {
        struct {
          void *p;
          uint32_t u32;
        } x_endndx;
      } x_fcn;
    } x_fcnary;
    struct {
      void *p;
      uint32_t u32;
    } x_tagndx;
  } x_sym;
  struct {
    struct {
      void *p;
      uint64_t u64;
    } x_scnlen;
  } x_csect;
  char pad[256];
};

struct internal_syment {
  int n_numaux;
  uintptr_t n_value;
};

struct combined_entry_type {
  bool is_sym;        /* True for symbol entries, false for aux entries. */
  int fix_tag;
  int fix_end;
  int fix_scnlen;
  union {
    struct internal_syment syment;   /* when is_sym == true */
    union internal_auxent auxent;    /* when is_sym == false */
  } u;
};

struct coff_symbol_type {
  struct combined_entry_type *native; /* Points into an array: [syment][aux...]. */
};

struct asymbol {
  struct coff_symbol_type *csym;
};

static inline struct coff_symbol_type *coff_symbol_from(struct asymbol *sym) {
  return sym ? sym->csym : NULL;
}

/* Vulnerable function (adapted from the provided source). */
bool bfd_coff_get_auxent(bfd *abfd,
                         struct asymbol *symbol,
                         int indx,
                         union internal_auxent *pauxent)
{
  struct coff_symbol_type *csym;
  struct combined_entry_type *ent;

  csym = coff_symbol_from(symbol);

  if (csym == NULL
      || csym->native == NULL
      || ! csym->native->is_sym
      || indx >= csym->native->u.syment.n_numaux)
    {
      bfd_set_error(bfd_error_invalid_operation);
      return false;
    }

  /* BUG: negative indx not rejected, so this can point before start. */
  ent = csym->native + indx + 1;

  /* In upstream this asserts it's an auxent. We make it a no-op so we reach the OOB read below. */
  BFD_ASSERT(! ent->is_sym);

  /* Out-of-bounds read happens here when ent points before the allocated array. */
  *pauxent = ent->u.auxent;

  if (ent->fix_tag)
    {
      pauxent->x_sym.x_tagndx.u32 =
        ((struct combined_entry_type *) pauxent->x_sym.x_tagndx.p
         - obj_raw_syments(abfd));
      ent->fix_tag = 0;
    }

  if (ent->fix_end)
    {
      pauxent->x_sym.x_fcnary.x_fcn.x_endndx.u32 =
        ((struct combined_entry_type *) pauxent->x_sym.x_fcnary.x_fcn.x_endndx.p
         - obj_raw_syments(abfd));
      ent->fix_end = 0;
    }

  if (ent->fix_scnlen)
    {
      pauxent->x_csect.x_scnlen.u64 =
        ((struct combined_entry_type *) pauxent->x_csect.x_scnlen.p
         - obj_raw_syments(abfd));
      ent->fix_scnlen = 0;
    }

  return true;
}

int main(void)
{
  /* Allocate a single symbol entry. There are no allocated aux entries
     following it, so any attempt to access an auxent at a negative index
     will read outside the allocated buffer. */
  struct combined_entry_type *arr = (struct combined_entry_type *)malloc(sizeof(*arr) * 1);
  if (!arr) {
    perror("malloc");
    return 1;
  }

  /* Set up the symbol entry (index 0). */
  arr[0].is_sym = true;                 /* It is a symbol entry. */
  arr[0].u.syment.n_numaux = 1;         /* One auxent expected (but we didn't allocate it). */
  arr[0].fix_tag = 0;
  arr[0].fix_end = 0;
  arr[0].fix_scnlen = 0;

  struct coff_symbol_type cs;
  cs.native = arr;                       /* Points at the symbol entry. */

  struct asymbol sym;
  sym.csym = &cs;

  bfd fake_bfd = {0};
  union internal_auxent out;

  /* Pass a negative index to trigger the bug. indx = -2 makes ent = native + (-2) + 1 = native - 1
     which points before the start of the allocated array, causing an OOB read. */
  int indx = -2;

  /* This call should trigger an ASan heap-buffer-overflow read. */
  (void)bfd_coff_get_auxent(&fake_bfd, &sym, indx, &out);

  /* Clean up (likely not reached if ASan aborts). */
  free(arr);
  return 0;
}
