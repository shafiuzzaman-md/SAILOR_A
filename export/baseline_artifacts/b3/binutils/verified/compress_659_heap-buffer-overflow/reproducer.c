#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Self-contained reproducer for the heap-buffer-overflow in
 * bfd/compress.c:bfd_compress_section_contents when using ZSTD.
 *
 * We provide minimal stubs for the involved BFD types and helpers and a
 * stub ZSTD_compress that blindly writes dstCapacity bytes to dst.
 * This mirrors the vulnerable call where dst points to buffer+new_header_size
 * but the capacity passed in erroneously includes new_header_size. */

/* --- Minimal BFD-like scaffolding --- */
typedef size_t bfd_size_type;

typedef struct bfd {
  unsigned int flags;
} bfd;

typedef struct asection {
  void *contents;
  unsigned int flags;
  size_t size;
  int compress_status;
} asection;

/* Flags used by the real code */
#define BFD_COMPRESS_ZSTD 0x1

/* Stubs for helper functions used in the real code (no-ops here) */
static void *bfd_alloc(bfd *abfd, bfd_size_type sz) {
  (void)abfd;
  return malloc(sz);
}
static void bfd_release(bfd *abfd, void *p) {
  (void)abfd;
  free(p);
}
static void bfd_set_error(int err) { (void)err; }

/* Very simple compressBound stand-in; returns something larger than input */
static size_t compressBound(size_t n) {
  return n + 64; /* arbitrary overhead to mimic zlib's compressBound */
}

/* --- ZSTD stubs --- */
#define HAVE_ZSTD 1
#define ZSTD_CLEVEL_DEFAULT 3

/* Stub: Pretend-compress by writing exactly dstCapacity bytes to dst. */
static size_t ZSTD_compress(void *dst, size_t dstCapacity,
                            const void *src, size_t srcSize, int level) {
  (void)src; (void)srcSize; (void)level;
  /* Fill the entire advertised capacity; this will overflow if the caller
     overestimated capacity relative to the actual allocation behind dst. */
  memset(dst, 0xA5, dstCapacity);
  return dstCapacity; /* pretend we used the full capacity */
}
static int ZSTD_isError(size_t code) { (void)code; return 0; }

/* --- Vulnerable function replica (trimmed to the critical logic) --- */
static bfd_size_type bfd_compress_section_contents(bfd *abfd, asection *sec, int update) {
  /* Craft sizes to exercise the bug: non-zero header and !update path. */
  size_t uncompressed_size = 1024;
  size_t new_header_size = 128; /* header size > 0 is key to triggering the bug */

  /* Uncompressed input buffer (contents to compress) */
  unsigned char *input_buffer = (unsigned char *)malloc(uncompressed_size);
  if (!input_buffer) return (bfd_size_type)-1;
  /* Fill with some data */
  for (size_t i = 0; i < uncompressed_size; i++) input_buffer[i] = (unsigned char)(i & 0xFF);

  size_t compressed_size;
  if (!update) {
    /* Buggy logic from the real code: compressed_size already includes header. */
    compressed_size = compressBound(uncompressed_size) + new_header_size;
  } else {
    compressed_size = uncompressed_size; /* not used in this reproducer */
  }

  size_t buffer_size = compressed_size;
  unsigned char *buffer = (unsigned char *)bfd_alloc(abfd, buffer_size);
  if (buffer == NULL) {
    free(input_buffer);
    return (bfd_size_type)-1;
  }

  /* Enter the ZSTD branch: the vulnerable call passes capacity=compressed_size
     but the destination pointer is buffer+new_header_size, leaving only
     buffer_size - new_header_size bytes writable. */
  if (!update && (abfd->flags & BFD_COMPRESS_ZSTD)) {
#if HAVE_ZSTD
    /* Vulnerable call: dst = buffer + new_header_size (shifted),
       dstCapacity = compressed_size (includes header). This will cause the
       stub ZSTD_compress to write new_header_size bytes past the end. */
    compressed_size = ZSTD_compress(buffer + new_header_size,
                                    compressed_size, /* overstates capacity */
                                    input_buffer,
                                    uncompressed_size,
                                    ZSTD_CLEVEL_DEFAULT);
    if (ZSTD_isError(compressed_size)) {
      bfd_release(abfd, buffer);
      bfd_set_error(1);
      free(input_buffer);
      return (bfd_size_type)-1;
    }
#endif
  }

  /* Clean up to avoid leaks; overflow has already occurred. */
  bfd_release(abfd, buffer);
  free(input_buffer);

  /* Return dummy value */
  return (bfd_size_type)0;
}

int main(void) {
  bfd abfd;
  asection sec;
  memset(&abfd, 0, sizeof(abfd));
  memset(&sec, 0, sizeof(sec));

  /* Ensure we take the ZSTD path */
  abfd.flags = BFD_COMPRESS_ZSTD;

  /* Call with update = 0 to take the compression path that contains the bug. */
  (void)bfd_compress_section_contents(&abfd, &sec, 0);

  /* If ASan is enabled, it should report a heap-buffer-overflow here. */
  fprintf(stderr, "Done. If AddressSanitizer is enabled, an overflow should have been reported.\n");
  return 0;
}