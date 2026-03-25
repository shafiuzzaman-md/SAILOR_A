#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal stand-ins for BFD types and utilities used by xcoff_write_armap_big */

typedef unsigned long long bfd_vma;

typedef struct bfd {
  struct bfd *archive_head;
  struct bfd *archive_next;
} bfd;

typedef struct {
  int bits_per_address;
} bfd_arch_info_type;

struct xcoff_ar_hdr_big {
  char size[20];
  char nextoff[20];
  char prevoff[20];
  char date[12];
  char uid[12];
  char gid[12];
  char mode[12];
  char namlen[4];
};

#define SIZEOF_AR_HDR_BIG (sizeof(struct xcoff_ar_hdr_big))
#define SXCOFFARFMAG 20
static const char XCOFFARFMAG[SXCOFFARFMAG] = {
  'A','R','M','A','P','_','B','I','G',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

/* PRINT macros: zero-out corresponding header fields (content not relevant for bug) */
#define PRINT20(field, val) do { (void)(val); memset((field), 0, sizeof(field)); } while (0)
#define PRINT12(field, val) do { (void)(val); memset((field), 0, sizeof(field)); } while (0)
#define PRINT4(field, val)  do { (void)(val); memset((field), 0, sizeof(field)); } while (0)

static void *bfd_zmalloc(size_t n) { return calloc(1, n); }
static void bfd_h_put_64(bfd *abfd, uint64_t val, void *dst) { (void)abfd; memcpy(dst, &val, 8); }
static bfd_vma bfd_write(const void *buf, bfd_vma size, bfd *abfd) { (void)buf; (void)abfd; return size; }

/* Map entry as used in the source snippet */
struct orl_entry {
  bfd *abfd;
  const char **name; /* not used in the first loop */
};

/* Archive iterator stubs */
struct archive_iterator {
  struct { bfd *member; uint64_t offset; } current;
  size_t idx;
};

static bfd *g_iter_member = NULL; /* The single iterator member we return */

static void archive_iterator_begin(struct archive_iterator *it, bfd *abfd) {
  (void)abfd;
  it->idx = 0;
}

static bool archive_iterator_next(struct archive_iterator *it) {
  if (it->idx == 0) {
    it->current.member = g_iter_member;
    it->current.offset = 0xDEADBEEFCAFEBABEULL;
    it->idx = 1;
    return true;
  }
  return false;
}

/* Architecture info stub: choose 32-bit to avoid unrelated buffer writes */
static bfd_arch_info_type g_arch32 = { 32 };
static bfd_arch_info_type *bfd_get_arch_info(bfd *member) { (void)member; return &g_arch32; }

/* Reimplementation of the vulnerable portion of xcoff_write_armap_big */
static bool xcoff_write_armap_big(bfd *abfd,
                                  struct orl_entry *map,
                                  size_t orl_count,
                                  uint64_t prevoff,
                                  uint64_t sym_64,
                                  size_t str_64)
{
  struct xcoff_ar_hdr_big *hdr;
  char *symbol_table;
  char *st;
  size_t i;
  struct archive_iterator iterator;
  bfd_arch_info_type *arch_info;

  bfd_vma symbol_table_size =
      SIZEOF_AR_HDR_BIG + SXCOFFARFMAG + 8 + 8 * sym_64 + str_64 + (str_64 & 1);

  symbol_table = (char *) bfd_zmalloc(symbol_table_size);
  if (symbol_table == NULL)
    return false;

  hdr = (struct xcoff_ar_hdr_big *) symbol_table;

  PRINT20(hdr->size, 8 + 8 * sym_64 + str_64 + (str_64 & 1));
  PRINT20(hdr->nextoff, 0);
  PRINT20(hdr->prevoff, prevoff);
  PRINT12(hdr->date, 0);
  PRINT12(hdr->uid, 0);
  PRINT12(hdr->gid, 0);
  PRINT12(hdr->mode, 0);
  PRINT4(hdr->namlen, 0);

  st = symbol_table + SIZEOF_AR_HDR_BIG;
  memcpy(st, XCOFFARFMAG, SXCOFFARFMAG);
  st += SXCOFFARFMAG;

  /* Number of 64-bit entries */
  bfd_h_put_64(abfd, sym_64, st);
  st += 8;

  /* Vulnerable loop over the 64-bit offsets */
  i = 0;
  archive_iterator_begin(&iterator, abfd);
  while (i < orl_count && archive_iterator_next(&iterator)) {
    arch_info = bfd_get_arch_info(iterator.current.member);
    /* BUG: Missing i < orl_count check in this inner while condition */
    while (map[i].abfd == iterator.current.member) {
      if (arch_info->bits_per_address == 64) {
        bfd_h_put_64(abfd, iterator.current.offset, st);
        st += 8;
      }
      i++;
    }
  }

  if (bfd_write(symbol_table, symbol_table_size, abfd) != symbol_table_size) {
    free(symbol_table);
    return false;
  }
  free(symbol_table);
  return true;
}

int main(void) {
  /* Prepare minimal inputs to force the OOB read in the inner while. */
  bfd archive = {0};

  /* orl_count = 1 so that after i++ the check reads map[1] (out of bounds). */
  size_t orl_count = 1;
  struct orl_entry *map = (struct orl_entry *) malloc(orl_count * sizeof(*map));
  if (!map) return 1;

  /* Single member used both in map[0] and by the iterator to match and increment i to orl_count. */
  bfd *member0 = (bfd *) malloc(sizeof(bfd));
  if (!member0) return 1;
  memset(member0, 0, sizeof(bfd));

  const char *name0 = "sym0";
  map[0].abfd = member0;
  map[0].name = &name0;

  /* Iterator returns exactly this same member, causing i to reach orl_count. */
  g_iter_member = member0;

  /* Parameters not affecting the bug path. */
  uint64_t prevoff = 0;
  uint64_t sym_64 = 0; /* keep 0 to avoid unrelated writes */
  size_t str_64 = 0;

  /* Trigger: the second evaluation of while(map[i].abfd == ...) reads past map[orl_count-1]. */
  (void)xcoff_write_armap_big(&archive, map, orl_count, prevoff, sym_64, str_64);

  free(member0);
  free(map);
  return 0;
}
