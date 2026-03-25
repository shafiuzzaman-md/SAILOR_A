#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD/COFF types to hit the vulnerable code path. */

typedef struct asection asection;
struct bfd { void *tdata; };
struct asection { struct bfd *owner; int gc_mark; };

/* obj_convert(abfd) returns an integer array used to index into symbols. */
#define obj_convert(abfd) ((int *) ((abfd)->tdata))

struct bfd_link_info { int dummy; };

/* Minimal link-hash types. Not actually used (we force the fallback path). */
struct bfd_link_hash_entry {
  int type;
  union { struct { void *link; } i; struct { asection *section; } def; } u;
};
struct coff_link_hash_entry { struct bfd_link_hash_entry root; };

/* Minimal symbol/native structures to match the expression in the vulnerable code. */
struct native_u { int syment; };
struct native { struct native_u u; };
struct asymbol { struct native *native; };

/* Relocation entry containing r_symndx. */
struct reloc_t { unsigned int r_symndx; };

/* Cookie passed into the vulnerable function. */
struct coff_reloc_cookie {
  struct coff_link_hash_entry **sym_hashes; /* array of size N */
  struct reloc_t *rel;                      /* relocation with r_symndx */
  struct asymbol *symbols;                  /* base of symbol array */
};

/* GC mark hook signature as used by the target function. */
typedef asection *(*coff_gc_mark_hook_fn)(asection *, struct bfd_link_info *,
                                          struct reloc_t *,
                                          struct coff_link_hash_entry *,
                                          void * /* &syment */);

/* Dummy hook so that the call site is evaluable. */
static asection *dummy_gc_mark_hook(asection *sec, struct bfd_link_info *info,
                                    struct reloc_t *rel,
                                    struct coff_link_hash_entry *h,
                                    void *syment)
{
  (void)sec; (void)info; (void)rel; (void)h; (void)syment;
  printf("dummy_gc_mark_hook called (should not reach here if ASan triggers earlier)\n");
  return NULL;
}

/* Vulnerable function reproduced with minimal dependencies. */
static asection *
_bfd_coff_gc_mark_rsec (struct bfd_link_info *info, asection *sec,
                        coff_gc_mark_hook_fn gc_mark_hook,
                        struct coff_reloc_cookie *cookie)
{
  struct coff_link_hash_entry *h;

  /* First index into sym_hashes by r_symndx. We'll ensure this is in-bounds
     and NULL so the fallback path is taken. */
  h = cookie->sym_hashes[cookie->rel->r_symndx];
  if (h != NULL)
  {
    /* Not used in this reproducer. */
    while (h->root.type == 1 || h->root.type == 2)
      h = (struct coff_link_hash_entry *) h->root.u.i.link;

    return (*gc_mark_hook)(sec, info, cookie->rel, h, NULL);
  }

  /* Fallback path: vulnerable access. The index cookie->rel->r_symndx is
     not bounds-checked against obj_convert(sec->owner) length, causing
     an out-of-bounds read. */
  return (*gc_mark_hook) (sec, info, cookie->rel, NULL,
                          &(cookie->symbols
                            + obj_convert (sec->owner)[cookie->rel->r_symndx])
                            ->native->u.syment);
}

int main(void)
{
  /* Prepare a tiny convert array (length 4). */
  struct bfd *abfd = (struct bfd *)calloc(1, sizeof(*abfd));
  if (!abfd) { perror("calloc bfd"); return 1; }
  int *convert = (int *)calloc(4, sizeof(int)); /* very small */
  if (!convert) { perror("calloc convert"); return 1; }
  /* All zeros are fine; we won't reach reading symbols if ASan trips earlier. */
  abfd->tdata = convert;

  /* Section owned by this bfd. */
  asection sec;
  memset(&sec, 0, sizeof(sec));
  sec.owner = abfd;

  /* Create a relocation with a very large r_symndx that is:
     - within sym_hashes (so we take the fallback path with h == NULL),
     - out-of-bounds for convert[] (to trigger the OOB read). */
  struct reloc_t rel;
  rel.r_symndx = 100; /* >= sym_hashes size, choose appropriately below */

  /* sym_hashes large enough so that sym_hashes[100] is in-bounds and NULL. */
  size_t sym_hashes_len = 200;
  struct coff_link_hash_entry **sym_hashes =
      (struct coff_link_hash_entry **)calloc(sym_hashes_len, sizeof(*sym_hashes));
  if (!sym_hashes) { perror("calloc sym_hashes"); return 1; }
  /* sym_hashes[] entries are already NULL from calloc. */

  /* Minimal symbols array; not used if ASan catches the OOB on convert[]. */
  struct asymbol *symbols = (struct asymbol *)calloc(1, sizeof(*symbols));
  if (!symbols) { perror("calloc symbols"); return 1; }
  symbols->native = (struct native *)calloc(1, sizeof(*symbols->native));
  if (!symbols->native) { perror("calloc native"); return 1; }

  struct coff_reloc_cookie cookie;
  cookie.sym_hashes = sym_hashes;
  cookie.rel = &rel;
  cookie.symbols = symbols;

  struct bfd_link_info info;
  memset(&info, 0, sizeof(info));

  printf("About to trigger out-of-bounds read on obj_convert()[r_symndx]...\n");
  /* This call should trigger an ASan report due to the OOB read of
     obj_convert(sec->owner)[rel.r_symndx] (convert[100] with length 4). */
  (void)_bfd_coff_gc_mark_rsec(&info, &sec, dummy_gc_mark_hook, &cookie);

  /* Cleanup (unlikely reached if ASan aborts earlier). */
  free(symbols->native);
  free(symbols);
  free(sym_hashes);
  free(convert);
  free(abfd);

  return 0;
}
