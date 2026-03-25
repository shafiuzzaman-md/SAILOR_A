#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal type aliases to match the original code */
typedef unsigned char uch;
typedef unsigned long ulg;

/* Minimal structures to satisfy field accesses */
typedef struct {
    uch r, g, b;
} RGB;

typedef struct {
    int type;
    int rgb1_min, rgb1_max;
    int rgb2_min, rgb2_max;
} BGPattern;

typedef struct {
    ulg height;
    ulg width;
} RPNG2Info;

/* Globals emulating the original environment */
int pat = 1;                 /* non-zero to use bgscale_default */
int bgscale_default = 0;     /* the trigger: makes bgscale become 0 when pat != 0 */
int bgscale = 0;
int bg_rowbytes = 0;
void *bg_data = NULL;

RGB rgb[4];                  /* color table used by bg[] indices */
BGPattern bg[2];             /* at least two patterns: 0 and 1 */
RPNG2Info rpng2_info;        /* image dimensions */

/* Function containing the vulnerable logic */
static void rpng2_x_reload_bg_image(void)
{
    char *dest;
    uch r1, r2, g1, g2, b1, b2;
    uch r1_inv, r2_inv, g1_inv, g2_inv, b1_inv, b2_inv;
    int xidx, yidx, yidx_max;
    int even_odd_vert, even_odd_horiz, even_odd;
    int invert_gradient2 = (bg[pat].type & 0x08);
    int invert_column;
    ulg i, row;

    bgscale = (pat == 0)? 8 : bgscale_default;   /* bgscale becomes 0 when pat != 0 and bgscale_default == 0 */
    yidx_max = bgscale - 1;

    /* Vertical gradient branch: (bg[pat].type & 0x07) == 0 */
    if ((bg[pat].type & 0x07) == 0) {
        uch r1_min  = rgb[bg[pat].rgb1_min].r;
        uch g1_min  = rgb[bg[pat].rgb1_min].g;
        uch b1_min  = rgb[bg[pat].rgb1_min].b;
        uch r2_min  = rgb[bg[pat].rgb2_min].r;
        uch g2_min  = rgb[bg[pat].rgb2_min].g;
        uch b2_min  = rgb[bg[pat].rgb2_min].b;
        int r1_diff = rgb[bg[pat].rgb1_max].r - r1_min;
        int g1_diff = rgb[bg[pat].rgb1_max].g - g1_min;
        int b1_diff = rgb[bg[pat].rgb1_max].b - b1_min;
        int r2_diff = rgb[bg[pat].rgb2_max].r - r2_min;
        int g2_diff = rgb[bg[pat].rgb2_max].g - g2_min;
        int b2_diff = rgb[bg[pat].rgb2_max].b - b2_min;

        for (row = 0;  row < rpng2_info.height;  ++row) {
            /* Divide-by-zero here when bgscale == 0 */
            yidx = (int)(row % bgscale);
            even_odd_vert = (int)((row / bgscale) & 1);

            /* The rest is not reached when the crash occurs, but kept for fidelity */
            r1 = (uch)(r1_min + (r1_diff * yidx) / yidx_max);
            g1 = (uch)(g1_min + (g1_diff * yidx) / yidx_max);
            b1 = (uch)(b1_min + (b1_diff * yidx) / yidx_max);
            r1_inv = (uch)(r1_min + (r1_diff * (yidx_max - yidx)) / yidx_max);
            g1_inv = (uch)(g1_min + (g1_diff * (yidx_max - yidx)) / yidx_max);
            b1_inv = (uch)(b1_min + (b1_diff * (yidx_max - yidx)) / yidx_max);

            r2 = (uch)(r2_min + (r2_diff * yidx) / yidx_max);
            g2 = (uch)(g2_min + (g2_diff * yidx) / yidx_max);
            b2 = (uch)(b2_min + (b2_diff * yidx) / yidx_max);
            r2_inv = (uch)(r2_min + (r2_diff * (yidx_max - yidx)) / yidx_max);
            g2_inv = (uch)(g2_min + (g2_diff * (yidx_max - yidx)) / yidx_max);
            b2_inv = (uch)(b2_min + (b2_diff * (yidx_max - yidx)) / yidx_max);

            dest = (char *)bg_data + row * bg_rowbytes;
            for (i = 0;  i < rpng2_info.width;  ++i) {
                even_odd_horiz = (int)((i / bgscale) & 1);
                even_odd = even_odd_vert ^ even_odd_horiz;
                invert_column = (even_odd_horiz && (bg[pat].type & 0x10));
                if (even_odd == 0) {        /* gradient #1 */
                    if (invert_column) {
                        *dest++ = r1_inv;
                        *dest++ = g1_inv;
                        *dest++ = b1_inv;
                    } else {
                        *dest++ = r1;
                        *dest++ = g1;
                        *dest++ = b1;
                    }
                } else {                    /* gradient #2 */
                    if ((invert_column && invert_gradient2) || (!invert_column && !invert_gradient2)) {
                        *dest++ = r2;       /* not inverted or doubly inverted */
                        *dest++ = g2;
                        *dest++ = b2;
                    } else {
                        *dest++ = r2_inv;
                        *dest++ = g2_inv;
                        *dest++ = b2_inv;
                    }
                }
            }
        }
    }
}

int main(void)
{
    /* Set up minimal valid environment */
    rpng2_info.width = 1;
    rpng2_info.height = 1;              /* at least one row to enter the loop */

    bg_rowbytes = 3;                    /* 3 bytes per pixel (RGB) */
    bg_data = malloc(rpng2_info.height * bg_rowbytes);
    if (!bg_data) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    /* Initialize a small RGB palette */
    rgb[0].r = 0;   rgb[0].g = 0;   rgb[0].b = 0;
    rgb[1].r = 255; rgb[1].g = 0;   rgb[1].b = 0;
    rgb[2].r = 0;   rgb[2].g = 255; rgb[2].b = 0;
    rgb[3].r = 0;   rgb[3].g = 0;   rgb[3].b = 255;

    /* Configure pattern 1 to select the vertical gradient branch */
    pat = 1;                        /* ensure pat != 0 */
    bg[1].type = 0;                 /* (type & 0x07) == 0 => vertical gradient */
    bg[1].rgb1_min = 0; bg[1].rgb1_max = 1;
    bg[1].rgb2_min = 2; bg[1].rgb2_max = 3;

    /* Critical trigger: bgscale_default == 0 and pat != 0 => bgscale becomes 0 */
    bgscale_default = 0;

    /* This call will crash with integer divide-by-zero at yidx = row % bgscale */
    fprintf(stderr, "Triggering rpng2_x_reload_bg_image divide-by-zero...\n");
    rpng2_x_reload_bg_image();

    /* Not reached */
    free(bg_data);
    return 0;
}
