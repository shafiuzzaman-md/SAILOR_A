#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for libpng types/APIs so we can compile/link standalone */
typedef unsigned char png_byte;
typedef png_byte* png_bytep;
typedef uint32_t png_uint_32;

typedef struct png_struct_def png_struct;  /* opaque in real libpng */
typedef struct png_info_def png_info;      /* opaque in real libpng */
struct png_struct_def { int dummy; };
struct png_info_def  { int dummy; };

/* Stubs for the libpng read path used by the buggy function */
static void png_read_info(png_struct* png_ptr, png_info* info_ptr) {
  (void)png_ptr; (void)info_ptr;
}

static size_t png_get_rowbytes(png_struct* png_ptr, png_info* info_ptr) {
  (void)png_ptr; (void)info_ptr;
  /* Pick a reasonably sized row so the write is noticeable */
  return 256;
}

static png_uint_32 png_get_image_height(png_struct* png_ptr, png_info* info_ptr) {
  (void)png_ptr; (void)info_ptr;
  return 2; /* two rows */
}

static int png_set_interlace_handling(png_struct* png_ptr) {
  (void)png_ptr;
  return 1; /* single pass */
}

static void png_start_read_image(png_struct* png_ptr) {
  (void)png_ptr;
}

static void png_error(png_struct* png_ptr, const char* msg) {
  (void)png_ptr;
  fprintf(stderr, "png_error: %s\n", msg ? msg : "(null)");
  /* In libpng this does not return; we exit to simulate */
  exit(1);
}

/* Our stub will write 'rowbytes' bytes into both buffers to simulate decoding */
static void png_read_row(png_struct* png_ptr, png_bytep row, png_bytep display) {
  (void)png_ptr;
  size_t n = 256; /* must match png_get_rowbytes above */
  for (size_t i = 0; i < n; ++i) {
    row[i] = (png_byte)(i & 0xFF);
    if (display) {
      display[i] = (png_byte)(0xFF - (i & 0xFF));
    }
  }
}

/* Buggy function reproduced from contrib/libtests/timepng.c (trimmed) */
static void read_by_row(png_struct *png_ptr, png_info *info_ptr,
                        FILE *write_ptr, FILE *read_ptr)
{
  (void)write_ptr; (void)read_ptr; /* not used in this reproducer */

  /* BUG: 'display' is declared as a single byte, not a pointer */
  png_byte *row = NULL, display = 0;

  png_read_info(png_ptr, info_ptr);

  {
    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    row = (png_byte*)malloc(rowbytes);
    /* Truncates the pointer to a single byte! */
    display = (png_byte)(uintptr_t)malloc(rowbytes);

    if (row == NULL || display == 0)
      png_error(png_ptr, "OOM allocating row buffers");

    {
      png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
      int passes = png_set_interlace_handling(png_ptr);
      int pass;

      png_start_read_image(png_ptr);

      for (pass = 0; pass < passes; ++pass)
      {
        png_uint_32 y = height;
        while (y-- > 0) {
          /* The 'display' byte value is implicitly converted to a pointer here */
          png_read_row(png_ptr, row, (png_bytep)(uintptr_t)display);
        }
      }
    }
  }
}

int main(void) {
  png_struct png;
  png_info info;

  /* Call the buggy function; our stubs will cause writes into the bogus pointer */
  read_by_row(&png, &info, NULL, NULL);

  /* If we somehow survived, indicate completion */
  puts("Done (unexpected)");
  return 0;
}
