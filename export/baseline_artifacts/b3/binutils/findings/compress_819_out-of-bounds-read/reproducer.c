#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Minimal typedefs to mirror BFD types. */
typedef unsigned char bfd_byte;
typedef size_t bfd_size_type;

/* Dummy bfd and asection structs with only needed fields. */
typedef struct bfd {
  int dummy;
} bfd;

enum compress_status_enum {
  COMPRESS_SECTION_NONE = 0,
  DECOMPRESS_SECTION_ZLIB = 1,
  DECOMPRESS_SECTION_ZSTD = 2,
  COMPRESS_SECTION_DONE = 3
};

typedef struct asection {
  bfd_size_type compressed_size;
  bfd_size_type rawsize;
  bfd_size_type size;
  int compress_status;
  bfd_byte *contents;
} asection;

/* Stubs mimicking BFD helper APIs used by the vulnerable code. */
static void *bfd_malloc(bfd_size_type n) { return malloc(n); }

enum bfd_error_enum { bfd_error_bad_value = 1 };
static void bfd_set_error(enum bfd_error_enum e) { (void)e; }

/* Return a compression header size that is larger than compressed_size. */
static bfd_size_type bfd_get_compression_header_size(bfd *abfd, asection *sec) {
  (void)abfd; (void)sec;
  /* Force a header larger than our test compressed_size to trigger OOB. */
  return 16;
}

/* Pretend to read section contents into the provided buffer. */
static bool bfd_get_section_contents(bfd *abfd, asection *sec,
                                     void *buf, bfd_size_type offset,
                                     bfd_size_type count) {
  (void)abfd; (void)sec; (void)offset; (void)count;
  /* Fill with a known byte. */
  if (count > 0 && buf != NULL) {
    ((bfd_byte*)buf)[0] = 0xAA;
  }
  return true;
}

/* Sink to prevent compiler from optimizing away reads. */
static volatile unsigned char g_sink;

/* Stub "decompression" that touches the first byte of the input buffer. */
static bool decompress_contents(bool is_zstd,
                                const bfd_byte *in, bfd_size_type in_len,
                                bfd_byte *out, bfd_size_type out_len) {
  (void)is_zstd; (void)out; (void)out_len;
  /* This read will be out-of-bounds if 'in' points past the allocation. */
  if (in_len == 0) {
    /* Even with zero length, the pointer may already be OOB. Force a read. */
    g_sink ^= in[0];
  } else {
    g_sink ^= in[0];
  }
  return true;
}

/* Vulnerable function logic (trimmed to the relevant case). */
bool bfd_get_full_section_contents(bfd *abfd, asection *sec, void **ptr) {
  bfd_byte *compressed_buffer;
  bfd_byte *p = NULL;
  bfd_size_type allocsz = 32;   /* arbitrary output buffer size */
  bfd_size_type readsz = 1;     /* we only need to read 1 byte to show OOB */
  bfd_size_type save_rawsize, save_size;
  int compress_status = sec->compress_status;
  bool ret;

  switch (compress_status) {
    case DECOMPRESS_SECTION_ZLIB:
    case DECOMPRESS_SECTION_ZSTD: {
      /* Read in the full compressed section contents. */
      compressed_buffer = (bfd_byte *) bfd_malloc(sec->compressed_size);
      if (compressed_buffer == NULL)
        return false;
      save_rawsize = sec->rawsize;
      save_size = sec->size;
      /* Clear rawsize, set size to compressed size and set compress_status
         to COMPRESS_SECTION_NONE. */
      sec->rawsize = 0;
      sec->size = sec->compressed_size;
      sec->compress_status = COMPRESS_SECTION_NONE;
      ret = bfd_get_section_contents(abfd, sec, compressed_buffer,
                                     0, sec->compressed_size);
      /* Restore rawsize and size. */
      sec->rawsize = save_rawsize;
      sec->size = save_size;
      sec->compress_status = compress_status;
      if (!ret)
        goto fail_compressed;

      if (p == NULL)
        p = (bfd_byte *) bfd_malloc(allocsz);
      if (p == NULL)
        goto fail_compressed;

      bfd_size_type compression_header_size = bfd_get_compression_header_size(abfd, sec);
      if (compression_header_size == 0)
        /* Set header size to the zlib header size if it is a SHF_COMPRESSED section. */
        compression_header_size = 12;
      bool is_zstd = (compress_status == DECOMPRESS_SECTION_ZSTD);

      /* BUG: No check that sec->compressed_size >= compression_header_size */
      if (!decompress_contents(
              is_zstd, compressed_buffer + compression_header_size,
              sec->compressed_size - compression_header_size, p, readsz)) {
        bfd_set_error(bfd_error_bad_value);
        if (p != (bfd_byte *)*ptr)
          free(p);
      fail_compressed:
        free(compressed_buffer);
        return false;
      }

      free(compressed_buffer);
      *ptr = p;
      return true;
    }

    default:
      return false;
  }
}

int main(void) {
  bfd ab;
  asection sec;
  memset(&ab, 0, sizeof(ab));
  memset(&sec, 0, sizeof(sec));

  /* Craft a section with a very small compressed_size so that
     compression_header_size (stub returns 16) exceeds it. */
  sec.compressed_size = 1;       /* smaller than header size (16) */
  sec.rawsize = 0;
  sec.size = sec.compressed_size;
  sec.compress_status = DECOMPRESS_SECTION_ZLIB; /* choose zlib path */
  sec.contents = NULL;

  void *out_ptr = NULL;

  /* This call will compute 'compressed_buffer + 16' with only 1 byte allocated
     and then attempt to read from it inside decompress_contents, triggering
     an out-of-bounds read under ASan. */
  (void)bfd_get_full_section_contents(&ab, &sec, &out_ptr);

  /* Clean up if anything was allocated before ASan aborts (best effort). */
  free(out_ptr);
  return 0;
}
