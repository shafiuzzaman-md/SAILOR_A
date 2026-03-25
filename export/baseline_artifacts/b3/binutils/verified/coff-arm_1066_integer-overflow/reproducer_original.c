#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Stubs and minimal re-declarations to make the reproducer self-contained. */
typedef struct bfd_link_info { int dummy; } bfd_link_info;
typedef struct bfd { int dummy; } bfd;
typedef struct coff_link_hash_entry { int dummy; } coff_link_hash_entry;

/* Vulnerable format string used by the real code. */
#define ARM2THUMB_GLUE_ENTRY_NAME "__%s_from_arm"

/* bfd malloc stub */
static void *bfd_malloc(size_t amt) { return malloc(amt); }

/* BFD_ASSERT stub */
#define BFD_ASSERT(x) do { if (!(x)) { fprintf(stderr, "BFD_ASSERT failed at %s:%d\n", __FILE__, __LINE__); abort(); } } while (0)

/* Hash table and lookup stubs - not relevant to triggering the overflow. */
static void *coff_hash_table(struct bfd_link_info *info) { (void)info; return NULL; }
static struct coff_link_hash_entry *coff_link_hash_lookup(void *table, const char *name,
                                                         bool create, bool copy, bool follow)
{
  (void)table; (void)name; (void)create; (void)copy; (void)follow;
  /* Return non-NULL so we don't hit the error handler path. */
  static struct coff_link_hash_entry dummy;
  return &dummy;
}

/* Error handler stub. */
static void _bfd_error_handler(const char *fmt, ...) { (void)fmt; }

/* We override strlen in the vulnerable function to simulate the integer overflow
   condition without requiring an impossible, near-SIZE_MAX input string. */
static const char *g_long_name = NULL; /* Pointer to the long name buffer we pass in. */

static size_t libc_strlen_fallback(const char *s) {
  size_t n = 0; if (!s) return 0; while (s[n] != '\0') n++; return n;
}

/* Macro so the vulnerable function uses our strlen. */
#define strlen my_strlen
static size_t my_strlen(const char *s) {
  if (s == g_long_name) {
    /* Craft a value so that: amt = strlen(name) + strlen(fmt) + 1 wraps
       to a very small number (e.g., 16). */
    size_t fmt_len = libc_strlen_fallback(ARM2THUMB_GLUE_ENTRY_NAME);
    size_t small = 16; /* final wrapped size after overflow */
    return (size_t)(~(size_t)0) - fmt_len - 1 + small; /* SIZE_MAX - fmt_len - 1 + small */
  }
  return libc_strlen_fallback(s);
}

/* Vulnerable function reproduced from bfd/coff-arm.c (reduced context). */
static struct coff_link_hash_entry *
find_arm_glue(struct bfd_link_info *info, const char *name, bfd *input_bfd)
{
  char *tmp_name;
  struct coff_link_hash_entry *myh;
  size_t amt = strlen(name) + strlen(ARM2THUMB_GLUE_ENTRY_NAME) + 1;

  tmp_name = bfd_malloc(amt);

  BFD_ASSERT(tmp_name);

  /* Heap buffer overflow happens here because amt wrapped to a tiny value
     but sprintf writes the full formatted string. */
  sprintf(tmp_name, ARM2THUMB_GLUE_ENTRY_NAME, name);

  myh = coff_link_hash_lookup(coff_hash_table(info), tmp_name, false, false, true);

  if (myh == NULL)
    _bfd_error_handler("%pB: unable to find ARM glue '%s' for `%s'", input_bfd, tmp_name, name);

  free(tmp_name);
  return myh;
}

int main(void)
{
  /* Create a large (but reasonable) real name so that sprintf writes a lot. */
  const size_t real_len = 100000; /* 100 KB name */
  char *name = (char *)malloc(real_len + 1);
  if (!name) {
    fprintf(stderr, "malloc failed\n");
    return 1;
  }
  memset(name, 'A', real_len);
  name[real_len] = '\0';

  g_long_name = name; /* Mark this pointer so my_strlen lies about its length. */

  struct bfd_link_info info = {0};
  bfd input = {0};

  /* This call will compute a wrapped allocation size (tiny) and then sprintf
     will overflow that heap buffer writing the long formatted string. */
  (void)find_arm_glue(&info, name, &input);

  free(name);
  return 0;
}
