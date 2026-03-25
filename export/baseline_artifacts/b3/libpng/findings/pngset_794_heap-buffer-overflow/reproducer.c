#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal re-declarations to mirror the vulnerable logic in libpng's png_set_PLTE */

typedef unsigned char png_byte;

typedef struct {
  png_byte red;
  png_byte green;
  png_byte blue;
} png_color;

/* Constants mirroring libpng */
#define PNG_COLOR_TYPE_PALETTE 3
#define PNG_MAX_PALETTE_LENGTH 256
#define PNG_FREE_PLTE 0x0001
#define PNG_INFO_PLTE 0x0001

/* Forward declarations */
struct png_struct_def;
struct png_info_def;

typedef struct png_struct_def {
  png_color *palette;
  unsigned short num_palette;
} png_struct;

typedef struct png_info_def {
  unsigned int color_type;
  unsigned int bit_depth; /* Intentionally wider than real libpng to allow >8 */
  png_color *palette;
  unsigned short num_palette;
  unsigned int free_me;
  unsigned int valid;
} png_info;

/* Stubs for libpng internals used by png_set_PLTE */
static void png_error(png_struct *png_ptr, const char *msg) {
  (void)png_ptr;
  fprintf(stderr, "png_error: %s\n", msg);
  /* In real libpng this would longjmp; for our purposes, abort to avoid hiding the bug */
  abort();
}

static void png_warning(png_struct *png_ptr, const char *msg) {
  (void)png_ptr;
  fprintf(stderr, "png_warning: %s\n", msg);
}

static void *png_calloc(png_struct *png_ptr, size_t size) {
  (void)png_ptr;
  void *p = calloc(1, size);
  if (!p) {
    fprintf(stderr, "Out of memory\n");
    abort();
  }
  return p;
}

static void png_free_data(png_struct *png_ptr, png_info *info_ptr, unsigned int mask, int num) {
  (void)png_ptr; (void)mask; (void)num;
  /* Free any earlier PLTE owned by info_ptr to keep this self-contained */
  if (info_ptr && info_ptr->palette) {
    free(info_ptr->palette);
    info_ptr->palette = NULL;
  }
}

/* Vulnerable function modeled after libpng png_set_PLTE around lines 754-801 */
static void png_set_PLTE(png_struct *png_ptr, png_info *info_ptr, const png_color *palette, int num_palette) {
  unsigned int max_palette_length;

  if (png_ptr == NULL || info_ptr == NULL) return;

  /* Compute max allowed length. If color type is PALETTE, use 1<<bit_depth. */
  if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) {
    /* The bug: bit_depth can be >8, so this becomes >256. */
    max_palette_length = 1U << info_ptr->bit_depth;
  } else {
    max_palette_length = PNG_MAX_PALETTE_LENGTH;
  }

  if (num_palette < 0 || (unsigned int)num_palette > max_palette_length) {
    if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_error(png_ptr, "Invalid palette length");
    else {
      png_warning(png_ptr, "Invalid palette length");
      return;
    }
  }

  if ((num_palette > 0 && palette == NULL) || (num_palette == 0)) {
    png_error(png_ptr, "Invalid palette");
  }

  /* Free any existing PLTE */
  png_free_data(png_ptr, info_ptr, PNG_FREE_PLTE, 0);

  /* Allocation is always for PNG_MAX_PALETTE_LENGTH (256) entries */
  png_ptr->palette = (png_color *)png_calloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));

  /* Vulnerable copy: if num_palette > 256 (due to large bit_depth), this overflows */
  if (num_palette > 0) {
    memcpy(png_ptr->palette, palette, (unsigned int)num_palette * sizeof(png_color));
  }

  info_ptr->palette = png_ptr->palette;
  info_ptr->num_palette = png_ptr->num_palette = (unsigned short)num_palette;
  info_ptr->free_me |= PNG_FREE_PLTE;
  info_ptr->valid |= PNG_INFO_PLTE;
}

int main(void) {
  png_struct png;
  png_info info;
  memset(&png, 0, sizeof(png));
  memset(&info, 0, sizeof(png_info));

  /* Set up conditions to pass the length check but overflow the destination */
  info.color_type = PNG_COLOR_TYPE_PALETTE;  /* Palette images */
  info.bit_depth = 16;                       /* Erroneously > 8, so max = 1<<16 */

  const int num_palette = 300;               /* > 256 triggers overflow */

  png_color *src = (png_color *)malloc((size_t)num_palette * sizeof(png_color));
  if (!src) {
    fprintf(stderr, "malloc failed\n");
    return 1;
  }

  /* Fill the source palette with deterministic data */
  for (int i = 0; i < num_palette; ++i) {
    src[i].red = (png_byte)(i & 0xFF);
    src[i].green = (png_byte)((i * 3) & 0xFF);
    src[i].blue = (png_byte)((i * 7) & 0xFF);
  }

  fprintf(stderr, "About to call png_set_PLTE with num_palette=%d and bit_depth=%u (expect ASan heap-buffer-overflow)\n",
          num_palette, info.bit_depth);

  /* This call should trigger the heap-buffer-overflow in memcpy */
  png_set_PLTE(&png, &info, src, num_palette);

  /* Touch the destination a bit so the compiler can't optimize things away */
  if (png.palette) {
    volatile unsigned sum = 0;
    for (int i = 0; i < num_palette; ++i) {
      sum += png.palette[i].red;
    }
    fprintf(stderr, "Checksum: %u\n", sum);
  }

  free(src);
  free(info.palette); /* In case it didn't crash before */
  return 0;
}
