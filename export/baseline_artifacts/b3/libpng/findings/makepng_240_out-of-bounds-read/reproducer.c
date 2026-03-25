#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal typedefs matching libpng expectations */
typedef unsigned char png_byte;
typedef struct { png_byte red, green, blue; } png_color;

/* Vulnerable function directly adapted from contrib/libtests/makepng.c */
static void set_color(png_color *color, png_byte *trans,
                      unsigned int red, unsigned int green, unsigned int blue,
                      unsigned int alpha, const png_byte *gamma_table)
{
    /* Out-of-bounds read when red/green/blue > 255 because gamma_table has 256 entries */
    color->red   = gamma_table[red];
    color->green = gamma_table[green];
    color->blue  = gamma_table[blue];
    *trans = (png_byte)alpha;
}

/* Palette generator that forwards user-controlled colors[] into set_color */
static int generate_palette(png_color *palette, png_byte *trans, int bit_depth,
                            const png_byte *gamma_table, unsigned int *colors)
{
    (void)bit_depth; /* unused in this minimal reproducer */

    switch (colors[0])
    {
    default:
        fprintf(stderr, "makepng: --colors=...: invalid count %u\n", colors[0]);
        exit(1);

    case 1:
        set_color(palette+0, trans+0, colors[1], colors[1], colors[1], 255, gamma_table);
        return 1;

    case 2:
        set_color(palette+0, trans+0, colors[1], colors[1], colors[1], colors[2], gamma_table);
        return 1;

    case 3:
        /* This path will perform three OOB reads when colors[1..3] > 255 */
        set_color(palette+0, trans+0, colors[1], colors[2], colors[3], 255, gamma_table);
        return 1;
    }
}

int main(void)
{
    /* Create a 256-entry gamma table as in typical libpng code paths */
    size_t gamma_entries = 256;
    png_byte *gamma_table = (png_byte *)malloc(gamma_entries);
    if (!gamma_table) {
        perror("malloc");
        return 1;
    }

    /* Fill with an identity mapping (0..255) */
    for (size_t i = 0; i < gamma_entries; ++i)
        gamma_table[i] = (png_byte)i;

    png_color palette[1];
    png_byte trans[1];

    /* colors[0] selects the case in generate_palette; colors[1..3] are components.
       Setting values >255 will index past the 256-entry gamma_table in set_color. */
    unsigned int colors[4];
    colors[0] = 3;     /* use the 3-component path */
    colors[1] = 300;   /* red   -> OOB read at gamma_table[300] */
    colors[2] = 400;   /* green -> OOB read at gamma_table[400] */
    colors[3] = 500;   /* blue  -> OOB read at gamma_table[500] */

    /* Trigger the vulnerability */
    (void)generate_palette(palette, trans, /*bit_depth=*/8, gamma_table, colors);

    /* Prevent compiler from optimizing away uses */
    printf("palette[0] = (%u,%u,%u), trans=%u\n",
           (unsigned)palette[0].red, (unsigned)palette[0].green,
           (unsigned)palette[0].blue, (unsigned)trans[0]);

    free(gamma_table);
    return 0;
}
