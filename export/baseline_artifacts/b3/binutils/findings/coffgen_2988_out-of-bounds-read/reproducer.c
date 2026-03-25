#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for types used by the vulnerable function. */

typedef struct asection {
  int gc_mark;
  void *owner;
} asection;

struct bfd_link_info { int dummy; };

/* Minimal stand-in for a coff relocation entry containing r_symndx. */
struct internal_reloc {
  size_t r_symndx;
};

/* Minimal stand-in for the COFF link hash entry. We don't need real fields
   for this reproducer because we only trigger the out-of-bounds access on
   cookie->sym_hashes[r_symndx]. */
struct coff_link_hash_entry {
  int dummy;
};

/* The relocation cookie containing the relocation and the sym_hashes array
   that is indexed by r_symndx. */
struct coff_reloc_cookie {
  struct internal_reloc *rel;
  struct coff_link_hash_entry **sym_hashes; /* array of pointers */
  void *symbols; /* unused in this reproducer */
};

/* Hook function type used by the real code; here it's a stub. */
typedef asection *(*coff_gc_mark_hook_fn)(asection *sec,
                                          struct bfd_link_info *info,
                                          void *rel, /* we use void* to keep it simple */
                                          void *h,
                                          void *syment);

static asection *dummy_gc_mark_hook(asection *sec,
                                    struct bfd_link_info *info,
                                    void *rel,
                                    void *h,
                                    void *syment)
{
  (void)sec; (void)info; (void)rel; (void)h; (void)syment;
  return NULL;
}

/* Simplified version of the vulnerable function _bfd_coff_gc_mark_rsec.
   It preserves the core buggy access: h = cookie->sym_hashes[cookie->rel->r_symndx];
   which lacks bounds checking on r_symndx. */
static asection *
_bfd_coff_gc_mark_rsec(struct bfd_link_info *info, asection *sec,
                       coff_gc_mark_hook_fn gc_mark_hook,
                       struct coff_reloc_cookie *cookie)
{
  (void)info; (void)sec;
  struct coff_link_hash_entry *h;

  /* Vulnerable out-of-bounds read: r_symndx not validated against
     the number of entries in cookie->sym_hashes. */
  h = cookie->sym_hashes[cookie->rel->r_symndx];

  /* Use the value in a way that prevents the compiler from optimizing the read away. */
  if (h) {
    return gc_mark_hook(sec, info, (void*)cookie->rel, h, NULL);
  } else {
    return gc_mark_hook(sec, info, (void*)cookie->rel, NULL, NULL);
  }
}

int main(void)
{
  /* Set up minimal structures to reach the vulnerable access. */
  struct bfd_link_info info = {0};
  asection sec = {0};

  /* Create a cookie with a small sym_hashes array (size 4). */
  struct coff_reloc_cookie cookie;
  memset(&cookie, 0, sizeof(cookie));

  size_t nsyms = 4; /* Intentionally small to trigger OOB when indexed with 5. */
  cookie.sym_hashes = (struct coff_link_hash_entry **)malloc(nsyms * sizeof(*cookie.sym_hashes));
  if (!cookie.sym_hashes) {
    perror("malloc sym_hashes");
    return 1;
  }
  for (size_t i = 0; i < nsyms; i++) cookie.sym_hashes[i] = NULL;

  cookie.rel = (struct internal_reloc *)malloc(sizeof(*cookie.rel));
  if (!cookie.rel) {
    perror("malloc rel");
    return 1;
  }

  /* Craft the malformed relocation: r_symndx beyond the array bounds. */
  cookie.rel->r_symndx = 5; /* OOB: valid indices are 0..3 */

  /* Call the vulnerable function: this will read past the end of sym_hashes. */
  (void)_bfd_coff_gc_mark_rsec(&info, &sec, dummy_gc_mark_hook, &cookie);

  /* Cleanup (normally unreachable if ASan aborts on OOB). */
  free(cookie.rel);
  free(cookie.sym_hashes);

  return 0;
}
