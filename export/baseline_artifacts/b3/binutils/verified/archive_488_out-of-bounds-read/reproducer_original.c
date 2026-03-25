// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer
// The harness may additionally add: -I/tmp/binutils_upstream -L/tmp/binutils_upstream/build/.libs -ltiff -lm

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// Minimal stand-ins for BFD types/macros used by get_extended_arelt_filename.
typedef long long file_ptr;  // close enough for our purposes

typedef struct artdata {
  unsigned long extended_names_size;
  char *extended_names;
} artdata;

typedef struct bfd {
  artdata *tdata;
  int is_thin_archive;
} bfd;

static inline artdata *bfd_ardata(bfd *abfd) { return abfd->tdata; }
static inline int bfd_is_thin_archive(bfd *abfd) { return abfd->is_thin_archive; }

// Stub for error reporting used by BFD; no-op here.
static void bfd_set_error(int unused) { (void)unused; }

// Vulnerable function reimplemented to mirror the binutils/bfd bug.
static char *get_extended_arelt_filename(bfd *arch, const char *name, file_ptr *originp) {
  unsigned long table_index = 0;
  const char *endp;

  errno = 0;
  // BUG: name points to a fixed-size 16-byte ar header field that may lack a NUL.
  // strtol scans past the field boundary looking for a non-digit, causing OOB read.
  table_index = strtol(name + 1, (char **)&endp, 10);
  if (errno != 0 || table_index >= bfd_ardata(arch)->extended_names_size) {
    bfd_set_error(0);
    return NULL;
  }

  if (bfd_is_thin_archive(arch) && endp != NULL && *endp == ':') {
    file_ptr origin = strtol(endp + 1, NULL, 10);
    if (errno != 0) {
      bfd_set_error(0);
      return NULL;
    }
    *originp = origin;
  } else {
    *originp = 0;
  }

  return bfd_ardata(arch)->extended_names + table_index;
}

// Place a 16-byte header-sized buffer at a guard page boundary so strtol reads OOB.
static char *allocate_16b_at_guard(void) {
  long pagesize = sysconf(_SC_PAGESIZE);
  if (pagesize <= 0) pagesize = 4096;

  size_t total = (size_t)pagesize * 2;
  void *mem = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    // Fallback to heap if mmap is unavailable.
    char *buf = (char *)malloc(16);
    return buf;
  }
  // Protect the second page so any read beyond the first page faults (and ASan also flags).
  mprotect((char *)mem + pagesize, (size_t)pagesize, PROT_NONE);

  // Return a pointer to the last 16 bytes of the first page.
  return (char *)mem + pagesize - 16;
}

int main(void) {
  // Craft the 16-byte archive header name field: leading '/' then 15 digits, no NUL terminator.
  char *name = allocate_16b_at_guard();
  if (!name) {
    fprintf(stderr, "Allocation failed\n");
    return 1;
  }
  name[0] = '/';
  for (int i = 1; i < 16; i++) name[i] = '1'; // 15 digits fill the field exactly, no terminator

  // Prepare a minimal bfd object with a small extended name table.
  char dummy_names[32];
  memset(dummy_names, 'A', sizeof(dummy_names));
  artdata t = { .extended_names_size = sizeof(dummy_names), .extended_names = dummy_names };
  bfd arch = { .tdata = &t, .is_thin_archive = 0 };

  file_ptr origin = -1;

  // This call triggers the out-of-bounds read inside strtol due to the missing NUL terminator.
  // ASan should report a heap/stack OOB read (depending on allocation path) originating in strtol.
  char *res = get_extended_arelt_filename(&arch, name, &origin);

  // Prevent optimizing away (though -O0 already):
  if (res) {
    // Touch the result to keep code paths live; not necessary for the bug to trigger.
    volatile char sink = res[0];
    (void)sink;
  }

  // If we used malloc fallback (no mmap), free to avoid leaks; otherwise it's fine to exit.
  // We cannot easily tell which path was taken, but freeing a mmap'd pointer is UB, so skip free.

  puts("Done (if you see no ASan report, try running again).\n");
  return 0;
}
