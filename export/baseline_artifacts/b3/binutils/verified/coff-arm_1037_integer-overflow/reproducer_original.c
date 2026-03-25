#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

/* ----------------- Minimal stubs and definitions ----------------- */

typedef struct bfd_link_info {
  int dummy;
} bfd_link_info;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct coff_link_hash_entry {
  int dummy;
} coff_link_hash_entry;

#define BFD_ASSERT(x) do { \
  if (!(x)) { \
    fprintf(stderr, "BFD_ASSERT failed at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
  } \
} while (0)

/* In real BFD this would be something like "__%s_from_thumb". */
#define THUMB2ARM_GLUE_ENTRY_NAME "GLUE_%s"

static void *bfd_malloc(size_t amt) {
  return malloc(amt);
}

static void *_coff_hash_table_placeholder = NULL;
static void *coff_hash_table(struct bfd_link_info *info) {
  (void)info;
  return _coff_hash_table_placeholder;
}

static struct coff_link_hash_entry *
coff_link_hash_lookup(void *table, const char *key, int create, int copy, int follow) {
  (void)table; (void)key; (void)create; (void)copy; (void)follow;
  return NULL; /* Not reached for the purposes of this reproducer. */
}

static void _bfd_error_handler(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fputc('\n', stderr);
  va_end(ap);
}

/* ----------------- Vulnerable function (as in bfd/coff-arm.c) ----------------- */

/* We override strlen used inside find_thumb_glue to simulate the integer overflow
   in the allocation size computation, without requiring an impossibly long input. */

static const char *g_malicious_name = NULL;

static size_t real_strlen(const char *s) {
  const char *p = s;
  while (*p) p++;
  return (size_t)(p - s);
}

static size_t my_fake_strlen(const char *s) {
  /* Return a huge value for the attacker's name to force size_t wraparound
     in: strlen(name) + strlen(THUMB2ARM_GLUE_ENTRY_NAME) + 1
     We choose (size_t)-4 (i.e., SIZE_MAX - 3). With format length 7 and +1,
     the sum becomes 4, causing a tiny allocation. */
  if (s == g_malicious_name) {
    return (size_t)-4; /* SIZE_MAX - 3 */
  }
  return real_strlen(s);
}

/* Redirect calls to strlen within this translation unit to our fake one. */
#define strlen my_fake_strlen

static struct coff_link_hash_entry *
find_thumb_glue(struct bfd_link_info *info,
                const char *name,
                bfd *input_bfd)
{
  char *tmp_name;
  struct coff_link_hash_entry *myh;
  size_t amt = strlen(name) + strlen(THUMB2ARM_GLUE_ENTRY_NAME) + 1;

  tmp_name = bfd_malloc(amt);

  BFD_ASSERT(tmp_name);

  /* Heap overflow here due to undersized allocation when amt wrapped. */
  sprintf(tmp_name, THUMB2ARM_GLUE_ENTRY_NAME, name);

  myh = coff_link_hash_lookup
    (coff_hash_table(info), tmp_name, 0, 0, 1);

  if (myh == NULL)
    _bfd_error_handler("%p: unable to find THUMB glue '%s' for `%s'",
                       (void *)input_bfd, tmp_name, name);

  free(tmp_name);
  return myh;
}

/* ----------------- Driver ----------------- */

int main(void) {
  /* Craft a reasonably sized name. The overflow will already occur when
     writing the literal "GLUE_" (5 bytes) into a 4-byte buffer. */
  size_t name_len = 64;
  char *name = (char *)malloc(name_len + 1);
  if (!name) {
    perror("malloc");
    return 1;
  }
  for (size_t i = 0; i < name_len; i++) name[i] = 'A';
  name[name_len] = '\0';

  g_malicious_name = name; /* Mark this pointer as the one to fake-length. */

  bfd_link_info info = {0};
  bfd input = {0};

  /* This call will trigger the heap overflow in sprintf due to the
     wrapped allocation size. With AddressSanitizer enabled, this should
     be reported immediately. */
  (void)find_thumb_glue(&info, name, &input);

  free(name);
  return 0;
}
