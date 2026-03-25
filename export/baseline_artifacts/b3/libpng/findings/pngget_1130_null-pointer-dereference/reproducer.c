#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal re-declarations to simulate libpng types and constants */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

typedef struct png_color_struct {
    png_byte red;
    png_byte green;
    png_byte blue;
} png_color;

typedef struct png_info_def {
    png_uint_32 valid;
    png_color *palette;
    int num_palette;
} png_info;

typedef struct png_struct_def {
    unsigned long chunk_name;
} png_struct;

/* Define the PLTE info bit (value chosen arbitrarily for this reproducer) */
#define PNG_INFO_PLTE 0x00000010u

/* Stub out debug macro used by the vulnerable function */
#define png_debug1(level, msg, arg) ((void)0)

/* Vulnerable function copied to reproduce the bug */
png_uint_32
png_get_PLTE(const png_struct *png_ptr, png_info *info_ptr,
             png_color **palette, int *num_palette)
{
    png_debug1(1, "in %s retrieval function", "PLTE");

    if (png_ptr != NULL && info_ptr != NULL &&
        (info_ptr->valid & PNG_INFO_PLTE) != 0 && palette != NULL)
    {
        *palette = info_ptr->palette;
        /* Bug: num_palette is used without a NULL check, causing NPD */
        *num_palette = info_ptr->num_palette;  /* NULL dereference if num_palette == NULL */
        png_debug1(3, "num_palette = %d", *num_palette);
        return PNG_INFO_PLTE;
    }

    return 0;
}

int main(void)
{
    /* Set up structures so that the condition passes and the bug is hit */
    png_struct png;               /* non-NULL png_ptr */
    png_info info;                /* non-NULL info_ptr */

    /* Create a small dummy palette */
    png_color *pal = (png_color *)malloc(3 * sizeof(png_color));
    if (!pal) return 1;
    pal[0].red = pal[0].green = pal[0].blue = 0;
    pal[1].red = pal[1].green = pal[1].blue = 127;
    pal[2].red = pal[2].green = pal[2].blue = 255;

    info.valid = PNG_INFO_PLTE;   /* Mark PLTE as present */
    info.palette = pal;           /* Provide a valid palette pointer */
    info.num_palette = 3;         /* Number of entries */

    png_color *palette_out = NULL; /* Output palette ptr location (non-NULL) */

    /* Trigger: pass num_palette == NULL to request only the palette */
    /* This causes png_get_PLTE to dereference NULL at *num_palette. */
    png_get_PLTE(&png, &info, &palette_out, NULL);

    /* Not reached due to crash */
    printf("palette_out=%p\n", (void *)palette_out);

    free(pal);
    return 0;
}
