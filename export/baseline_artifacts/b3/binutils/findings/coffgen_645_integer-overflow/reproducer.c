// Standalone C reproducer for the integer overflow in bfd_coff_read_internal_relocs
// Compile with: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Force 32-bit arithmetic for bfd_size_type to model the vulnerable build.
typedef uint32_t bfd_size_type;
typedef unsigned char bfd_byte;
typedef int bfd_boolean;

struct bfd { int dummy; };

struct internal_reloc {
  // Make this large so that amt = reloc_count * sizeof(struct internal_reloc)
  // overflows 32-bit easily and allocates too small a buffer.
  char pad[1024];
};

struct coff_section_tdata {
  struct internal_reloc *relocs;
  void *contents;
};

struct asection {
  bfd_size_type reloc_count;
  long rel_filepos;
  void *used_by_bfd; // will point to coff_section_tdata when allocated
};

static struct coff_section_tdata *coff_section_data(struct bfd *abfd, struct asection *sec) {
  (void)abfd;
  return (struct coff_section_tdata *)sec->used_by_bfd;
}

static bfd_size_type bfd_coff_relsz(struct bfd *abfd) {
  (void)abfd;
  // Keep external reloc size small so relsz * reloc_count does not wrap in 32-bit
  // for our chosen reloc_count, ensuring many loop iterations.
  return 2; // bytes per external reloc entry
}

static void *bfd_malloc(bfd_size_type amt) {
  return malloc((size_t)amt);
}

static void *bfd_zalloc(struct bfd *abfd, bfd_size_type amt) {
  (void)abfd;
  return calloc(1, (size_t)amt);
}

static int bfd_seek(struct bfd *abfd, long pos, int whence) {
  (void)abfd; (void)pos; (void)whence;
  return 0; // success
}

static bfd_size_type bfd_read(void *buf, bfd_size_type size, struct bfd *abfd) {
  (void)abfd;
  memset(buf, 0x41, (size_t)size);
  return size; // simulate a full read
}

// Stub that writes sizeof(struct internal_reloc) bytes into the destination,
// matching typical behavior of a swap-in routine that populates an internal structure.
static void bfd_coff_swap_reloc_in(struct bfd *abfd, void *erel, void *irel) {
  (void)abfd; (void)erel;
  memset(irel, 0x42, sizeof(struct internal_reloc));
}

// Vulnerable function recreated with stubs to trigger the overflow/overflowed allocation.
static struct internal_reloc *
bfd_coff_read_internal_relocs(struct bfd *abfd,
                              struct asection *sec,
                              bfd_byte *external_relocs,
                              struct internal_reloc *internal_relocs,
                              int cache,
                              int require_internal)
{
  bfd_size_type relsz;
  bfd_byte *free_external = NULL;
  struct internal_reloc *free_internal = NULL;
  bfd_byte *erel;
  bfd_byte *erel_end;
  struct internal_reloc *irel;
  bfd_size_type amt;

  if (sec->reloc_count == 0)
    return internal_relocs; /* Nothing to do.  */

  if (coff_section_data(abfd, sec) != NULL && coff_section_data(abfd, sec)->relocs != NULL) {
    if (!require_internal)
      return coff_section_data(abfd, sec)->relocs;
    memcpy(internal_relocs, coff_section_data(abfd, sec)->relocs,
           (size_t)(sec->reloc_count * (bfd_size_type)sizeof(struct internal_reloc)));
    return internal_relocs;
  }

  relsz = bfd_coff_relsz(abfd);

  amt = sec->reloc_count * relsz; // may overflow 32-bit if relsz large, here it does not
  if (external_relocs == NULL) {
    free_external = (bfd_byte *) bfd_malloc(amt);
    if (free_external == NULL)
      goto error_return;
    external_relocs = free_external;
  }

  if (bfd_seek(abfd, sec->rel_filepos, SEEK_SET) != 0
      || bfd_read(external_relocs, amt, abfd) != amt)
    goto error_return;

  if (internal_relocs == NULL) {
    amt = sec->reloc_count;
    amt *= (bfd_size_type)sizeof(struct internal_reloc); // integer overflow here on 32-bit
    free_internal = (struct internal_reloc *) bfd_malloc(amt);
    if (free_internal == NULL)
      goto error_return;
    internal_relocs = free_internal;
  }

  // Swap in the relocs.
  erel = external_relocs;
  erel_end = erel + (relsz * sec->reloc_count); // also uses 32-bit arithmetic here
  irel = internal_relocs;
  for (; erel < erel_end; erel += relsz, irel++)
    bfd_coff_swap_reloc_in(abfd, (void *) erel, (void *) irel);

  free(free_external);
  free_external = NULL;

  if (cache && free_internal != NULL) {
    if (coff_section_data(abfd, sec) == NULL) {
      amt = (bfd_size_type)sizeof(struct coff_section_tdata);
      sec->used_by_bfd = bfd_zalloc(abfd, amt);
      if (sec->used_by_bfd == NULL)
        goto error_return;
      coff_section_data(abfd, sec)->contents = NULL;
    }
    coff_section_data(abfd, sec)->relocs = free_internal;
  }

  return internal_relocs;

error_return:
  free(free_external);
  free(free_internal);
  return NULL;
}

int main(void) {
  struct bfd abfd_obj;
  struct asection sec;
  memset(&abfd_obj, 0, sizeof(abfd_obj));
  memset(&sec, 0, sizeof(sec));

  // Choose reloc_count so that:
  //  - amt_ext = reloc_count * relsz does NOT wrap (so loop iterates many times)
  //  - amt_int = reloc_count * sizeof(struct internal_reloc) DOES wrap to a tiny value
  // With bfd_size_type as 32-bit:
  //   sizeof(struct internal_reloc) = 1024
  //   2^32 / 1024 = 4,194,304
  // Set reloc_count = 4,194,304 + 1 = 4,194,305
  // Then:
  //   - external amt = 4,194,305 * 2 = 8,388,610 bytes (OK)
  //   - internal amt = (4,194,305 * 1024) mod 2^32 = 1024 bytes (only 1 entry)
  // The loop runs 4,194,305 iterations, writing 1024 bytes each time via bfd_coff_swap_reloc_in,
  // overflowing after the first iteration.
  sec.reloc_count = (bfd_size_type)(4194304u + 1u);
  sec.rel_filepos = 0;
  sec.used_by_bfd = NULL;

  struct internal_reloc *res = bfd_coff_read_internal_relocs(&abfd_obj, &sec,
                                                             NULL, NULL,
                                                             0, 0);
  // Prevent compiler from optimizing away side effects
  fprintf(stderr, "Result pointer: %p\n", (void*)res);

  // If the function returned (unlikely if ASan already crashed), free to avoid leaks
  free(res);
  return 0;
}
