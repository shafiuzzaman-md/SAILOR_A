#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal type aliases to mirror the original code's expectations */
typedef unsigned char uch;

/* Minimal RGB and background pattern structures */
typedef struct {
    uch r, g, b;
} RGB;

typedef struct {
    int type;       /* selects pattern branch; lower 3 bits used */
    int rgb1_max;   /* index into rgb[] for first color */
    int rgb2_max;   /* index into rgb[] for second color */
    /* other fields exist in the real project, but are not needed here */
} BGEntry;

/* Globals to mimic the environment of rpng2-x.c */
struct {
    unsigned int width;
    unsigned int height;
} rpng2_info;

static BGEntry bg[2];    /* we'll use pat = 1 */
static RGB rgb[2];       /* two colors referenced by rgb1_max/rgb2_max */
static void *bg_data;    /* destination buffer */
static size_t bg_rowbytes;
static int bgscale;      /* controls the gradient scale; set to 1 to trigger bug */

/* Reimplementation of the vulnerable branch in rpng2_x_reload_bg_image() */
static void rpng2_x_reload_bg_image(int pat)
{
    int hmax, max;
    int r1, g1, b1, r2, g2, b2;
    unsigned int row, i;
    int yidx, xidx, k;
    char *dest;

    /* Only the diamond-gradient branch, as in the original vulnerable code */
    if ((bg[pat].type & 0x07) == 1) {
        hmax = (bgscale - 1) / 2;  /* with bgscale = 1 -> hmax = 0 */
        max = 2 * hmax;            /* -> max = 0, causing division by zero below */

        r1 = rgb[bg[pat].rgb1_max].r;
        g1 = rgb[bg[pat].rgb1_max].g;
        b1 = rgb[bg[pat].rgb1_max].b;
        r2 = rgb[bg[pat].rgb2_max].r;
        g2 = rgb[bg[pat].rgb2_max].g;
        b2 = rgb[bg[pat].rgb2_max].b;

        for (row = 0; row < rpng2_info.height; ++row) {
            yidx = (int)(row % bgscale);             /* bgscale = 1 -> yidx = 0 */
            if (yidx > hmax)
                yidx = bgscale - 1 - yidx;
            dest = (char *)bg_data + row * bg_rowbytes;
            for (i = 0; i < rpng2_info.width; ++i) {
                xidx = (int)(i % bgscale);           /* bgscale = 1 -> xidx = 0 */
                if (xidx > hmax)
                    xidx = bgscale - 1 - xidx;
                k = xidx + yidx;                     /* k = 0 */
                /* The three divisions by max (which is 0) trigger SIGFPE */
                dest[0] = (k * r1 + (max - k) * r2) / max;  /* divide by zero */
                dest[1] = (k * g1 + (max - k) * g2) / max;  /* divide by zero */
                dest[2] = (k * b1 + (max - k) * b2) / max;  /* divide by zero */
                dest += 3;
            }
        }
    }
}

int main(void)
{
    /* Set up a 1x1 background buffer */
    rpng2_info.width = 1;
    rpng2_info.height = 1;
    bg_rowbytes = rpng2_info.width * 3; /* 3 bytes per pixel (RGB) */
    bg_data = malloc(bg_rowbytes * rpng2_info.height);
    if (!bg_data) {
        perror("malloc");
        return 1;
    }

    /* Configure bgscale to 1 to make max = 0 in the vulnerable code path */
    bgscale = 1;

    /* Choose pattern index pat = 1 (non-zero), type with low 3 bits == 1 to hit the diamond-gradient branch */
    int pat = 1;
    bg[pat].type = 1;  /* (type & 0x07) == 1 */
    bg[pat].rgb1_max = 0;
    bg[pat].rgb2_max = 1;

    /* Two arbitrary colors */
    rgb[0].r = 10;  rgb[0].g = 20;  rgb[0].b = 30;
    rgb[1].r = 200; rgb[1].g = 150; rgb[1].b = 100;

    /* This call will execute the diamond-gradient code and divide by zero */
    rpng2_x_reload_bg_image(pat);

    /* Not reached if SIGFPE occurs as expected */
    free(bg_data);
    return 0;
}
