#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Minimal type re-declarations and stubs to exercise the vulnerable loop. */
typedef unsigned long long bfd_vma;

struct bfd {
  struct bfd *archive_head;
  struct bfd *archive_next;
};

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

/* No-op macros for header printing used by the real code. */
#define PRINT20(field, value) do { (void)(field); (void)(value); } while (0)
#define PRINT12(field, value) do { (void)(field); (void)(value); } while (0)
#define PRINT4(field, value)  do { (void)(field); (void)(value); } while (0)

#define SIZEOF_AR_HDR_BIG (sizeof(struct xcoff_ar_hdr_big))
static const char XCOFFARFMAG[] = "ARFMAG";  /* Arbitrary content */
#define SXCOFFARFMAG (sizeof(XCOFFARFMAG) - 1)

static void *bfd_zmalloc(size_t sz) { return calloc(1, sz); }
static void bfd_h_put_64(struct bfd *abfd, uint64_t val, void *dst) {
  (void)abfd;
  memcpy(dst, &val, 8);
}

struct arch_info { int bits_per_address; };
static struct arch_info arch32 = { 32 };
static struct arch_info *bfd_get_arch_info(struct bfd *abfd) {
  (void)abfd;
  return &arch32;
}

struct archive_iterator {
  struct { struct bfd *member; uint64_t offset; } current;
  int idx;
};

static struct bfd global_member_bfd_instance;
static struct bfd *global_member_bfd = &global_member_bfd_instance;

static void archive_iterator_begin(struct archive_iterator *it, struct bfd *abfd) {
  (void)abfd;
  it->idx = 0;
  it->current.member = NULL;
  it->current.offset = 0;
}

static int archive_iterator_next(struct archive_iterator *it) {
  /* Return one member repeatedly; the bug triggers in the first iteration. */
  if (it->idx == 0) {
    it->current.member = global_member_bfd;
    it->current.offset = 0x1234ULL;
    it->idx++;
    return 1;
  }
  return 0;
}

static size_t bfd_write(const void *buf, size_t n, struct bfd *abfd) {
  (void)buf; (void)abfd;
  return n; /* Pretend write succeeds. */
}

struct orl_map_entry {
  struct bfd *abfd;
  const char **name;
};

/* Vulnerable function body distilled to the relevant part. */
static bool xcoff_write_armap_big(struct bfd *abfd, struct orl_map_entry *map, size_t orl_count) {
  size_t sym_32 = orl_count;
  size_t sym_64 = 0;
  size_t str_32 = 0;
  bfd_vma nextoff = 0, prevoff = 0;

  size_t symbol_table_size = SIZEOF_AR_HDR_BIG + SXCOFFARFMAG + 8 + 8 * sym_32 + str_32 + (str_32 & 1);
  char *symbol_table = (char *)bfd_zmalloc(symbol_table_size);
  if (!symbol_table) return false;

  struct xcoff_ar_hdr_big *hdr = (struct xcoff_ar_hdr_big *)symbol_table;
  PRINT20(hdr->size, 8 + 8 * sym_32 + str_32 + (str_32 & 1));
  if (sym_64)
    PRINT20(hdr->nextoff, nextoff + symbol_table_size);
  else
    PRINT20(hdr->nextoff, 0);
  PRINT20(hdr->prevoff, prevoff);
  PRINT12(hdr->date, 0);
  PRINT12(hdr->uid, 0);
  PRINT12(hdr->gid, 0);
  PRINT12(hdr->mode, 0);
  PRINT4(hdr->namlen, 0);

  char *st = symbol_table + SIZEOF_AR_HDR_BIG;
  memcpy(st, XCOFFARFMAG, SXCOFFARFMAG);
  st += SXCOFFARFMAG;
  bfd_h_put_64(abfd, sym_32, st);
  st += 8;

  /* Vulnerable region: missing bound check in inner while. */
  size_t i = 0;
  struct archive_iterator iterator;
  archive_iterator_begin(&iterator, abfd);
  while (i < orl_count && archive_iterator_next(&iterator)) {
    struct arch_info *arch_info = bfd_get_arch_info(iterator.current.member);
    while (map[i].abfd == iterator.current.member) { /* No i < orl_count check here -> OOB read when i == orl_count */
      if (arch_info->bits_per_address == 32) {
        bfd_h_put_64(abfd, iterator.current.offset, st);
        st += 8;
      }
      i++;
    }
  }

  /* Not reached if ASan trips as intended. */
  if (bfd_write(symbol_table, symbol_table_size, abfd) != symbol_table_size) {
    free(symbol_table);
    return false;
  }
  free(symbol_table);
  return true;
}

int main(void) {
  /* Set up a map with orl_count entries, all pointing to the same member. */
  const size_t orl_count = 8; /* Any positive number works. */
  struct orl_map_entry *map = (struct orl_map_entry *)malloc(orl_count * sizeof(*map));
  if (!map) {
    perror("malloc");
    return 1;
  }
  for (size_t i = 0; i < orl_count; i++) {
    map[i].abfd = global_member_bfd;  /* All equal to iterator.current.member. */
    map[i].name = NULL;               /* Not used before the bug triggers. */
  }

  struct bfd archive = {0};
  /* Call the vulnerable function; the inner while will increment i to orl_count
     and then re-evaluate map[i] with i == orl_count, causing an OOB read. */
  (void)xcoff_write_armap_big(&archive, map, orl_count);

  free(map);
  return 0;
}
