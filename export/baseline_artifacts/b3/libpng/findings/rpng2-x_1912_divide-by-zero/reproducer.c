#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type aliases mirroring the original code */
typedef unsigned char uch;
typedef unsigned long ulg;

/* Minimal RGB entry type */
typedef struct {
    uch r, g, b;
} rgb_t;

/* Background pattern description (subset needed by the vulnerable code) */
typedef struct {
    int type;        /* pattern type and flags */
    int rgb1_min;    /* index into rgb[] */
    int rgb1_max;    /* index into rgb[] */
    int rgb2_min;    /* index into rgb[] */
    int rgb2_max;    /* index into rgb[] */
} bg_t;

/* Globals mimicking those used by rpng2-x.c */
static struct {
    ulg width;
    ulg height;
} rpng2_info;

static void *bg_data = NULL;
static int bg_rowbytes = 0;
static int bgscale = 0;
static int bgscale_default = 1;  /* we will set pat != 0 so bgscale becomes 1 */
static int pat = 1;              /* choose non-zero pattern to hit the bug */

/* Minimal tables */
static rgb_t rgb[8];
static bg_t bg[4];

/* Vulnerable function (extracted/replicated from contrib/gregbook/rpng2-x.c) */
static void rpng2_x_reload_bg_image(void)
{
    char *dest;
    uch r1, r2, g1, g2, b1, b2;
    uch r1_inv, r2_inv, g1_inv, g2_inv, b1_inv, b2_inv;
    int k, hmax, max; /* unused but kept to preserve structure */
    int xidx, yidx, yidx_max;
    int even_odd_vert, even_odd_horiz, even_odd;
    int invert_gradient2 = (bg[pat].type & 0x08);
    int invert_column;
    ulg i, row;

    (void)k; (void)hmax; (void)max; (void)xidx; /* silence unused warnings */

    bgscale = (pat == 0) ? 8 : bgscale_default; /* when pat != 0, bgscale = bgscale_default */
    yidx_max = bgscale - 1;                     /* becomes 0 when bgscale == 1 */

    /* Vertical gradients (ramps) in NxN squares, alternating direction and colors */
    if ((bg[pat].type & 0x07) == 0) {
        uch r1_min  = rgb[bg[pat].rgb1_min].r;
        uch g1_min  = rgb[bg[pat].rgb1_min].g;
        uch b1_min  = rgb[bg[pat].rgb1_min].b;
        uch r2_min  = rgb[bg[pat].rgb2_min].r;
        uch g2_min  = rgb[bg[pat].rgb2_min].g;
        uch b2_min  = rgb[bg[pat].rgb2_min].b;
        int r1_diff = (int)rgb[bg[pat].rgb1_max].r - (int)r1_min;
        int g1_diff = (int)rgb[bg[pat].rgb1_max].g - (int)g1_min;
        int b1_diff = (int)rgb[bg[pat].rgb1_max].b - (int)b1_min;
        int r2_diff = (int)rgb[bg[pat].rgb2_max].r - (int)r2_min;
        int g2_diff = (int)rgb[bg[pat].rgb2_max].g - (int)g2_min;
        int b2_diff = (int)rgb[bg[pat].rgb2_max].b - (int)b2_min;

        for (row = 0; row < rpng2_info.height; ++row) {
            yidx = (int)(row % bgscale);            /* 0 when bgscale == 1 */
            even_odd_vert = (int)((row / bgscale) & 1);

            /* BUG: yidx_max == 0 when bgscale == 1, so the following divide-by-zero occurs */
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
            for (i = 0; i < rpng2_info.width; ++i) {
                even_odd_horiz = (int)((i / bgscale) & 1);
                even_odd = even_odd_vert ^ even_odd_horiz;
                invert_column = (even_odd_horiz && (bg[pat].type & 0x10));
                if (even_odd == 0) {
                    if (invert_column) {
                        *dest++ = r1_inv;
                        *dest++ = g1_inv;
                        *dest++ = b1_inv;
                    } else {
                        *dest++ = r1;
                        *dest++ = g1;
                        *dest++ = b1;
                    }
                } else {
                    if ((invert_column && invert_gradient2) || (!invert_column && !invert_gradient2)) {
                        *dest++ = r2;
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
    /* Configure image size and buffer */
    rpng2_info.width = 2;   /* small but non-zero */
    rpng2_info.height = 2;  /* small but non-zero */
    bg_rowbytes = (int)(rpng2_info.width * 3); /* RGB */
    bg_data = malloc(rpng2_info.height * bg_rowbytes);
    if (!bg_data) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    /* Set up RGB table entries referenced by bg[pat] */
    rgb[0].r = 0;   rgb[0].g = 0;   rgb[0].b = 0;     /* rgb1_min */
    rgb[1].r = 255; rgb[1].g = 0;   rgb[1].b = 0;     /* rgb1_max */
    rgb[2].r = 0;   rgb[2].g = 255; rgb[2].b = 0;     /* rgb2_min */
    rgb[3].r = 0;   rgb[3].g = 0;   rgb[3].b = 255;   /* rgb2_max */

    /* Configure background pattern: type selects vertical gradient branch ((type & 7) == 0) */
    pat = 1;                 /* non-zero, so bgscale = bgscale_default */
    bgscale_default = 1;     /* makes yidx_max = bgscale - 1 = 0 */

    bg[pat].type = 0;        /* ensures (type & 0x07) == 0 to enter vertical gradient path */
    bg[pat].rgb1_min = 0;
    bg[pat].rgb1_max = 1;
    bg[pat].rgb2_min = 2;
    bg[pat].rgb2_max = 3;

    /* This call will trigger an integer divide-by-zero at the first iteration */
    rpng2_x_reload_bg_image();

    /* If the bug did not trigger (it should), clean up */
    free(bg_data);
    puts("Unexpected: divide-by-zero did not occur");
    return 0;
}
