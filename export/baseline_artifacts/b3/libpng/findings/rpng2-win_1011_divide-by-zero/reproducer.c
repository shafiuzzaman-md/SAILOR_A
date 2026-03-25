#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

typedef unsigned char uch;

#define CLIP(a,l,h) ((a) < (l) ? (l) : ((a) > (h) ? (h) : (a)))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define PI 3.14159265358979323846
#define PI_2 (PI/2.0)
#define INV_PI_360 (360.0 / PI)

#define PROGNAME "repro"

/* Minimal stand-ins for the globals used by rpng2-win.c */
struct {
    int width;
    int height;
} rpng2_info;

struct bgconf {
    int type;     /* lower 3 bits select pattern (2 == radial) */
    int bg_gray;  /* gray spot size */
    int bg_freq;  /* frequency */
    int bg_bsat;  /* saturation*10 */
    int bg_brot;  /* rotation*10 */
};

static struct bgconf bg[1];

static void rpng2_win_load_bg_image(void)
{
    /* This function contains the vulnerable code path (radial background) */
    int pat = 0;

    if ((bg[pat].type & 0x07) == 2) {
        uch ch;
        int ii, x, y, hw, hh, grayspot;
        int row, i;
        double freq, rotate, saturate, gray, intensity;
        double angle = 0.0, aoffset = 0.0, maxDist, dist;
        double red = 0.0, green = 0.0, blue = 0.0, hue, s, v, f, p, q, t;

        fprintf(stderr, "%s:  computing radial background...\n", PROGNAME);
        fflush(stderr);

        hh = rpng2_info.height / 2;  /* for 1x1 image -> 0 */
        hw = rpng2_info.width / 2;   /* for 1x1 image -> 0 */

        /* Allocate a tiny destination buffer (RGB) */
        size_t bg_rowbytes = (size_t)rpng2_info.width * 3;
        size_t bufsz = (size_t)rpng2_info.height * bg_rowbytes;
        uch *bg_data = (uch*)calloc(1, bufsz ? bufsz : 1);
        uch *dest;
        if (!bg_data) {
            perror("calloc");
            exit(1);
        }

        angle = CLIP(angle, 0.0, 360.0);
        grayspot = CLIP(bg[pat].bg_gray, 1, (hh + hw));
        freq = MAX((double)bg[pat].bg_freq, 0.0);
        saturate = (double)bg[pat].bg_bsat * 0.1;
        rotate = (double)bg[pat].bg_brot * 0.1;
        gray = 0.0;
        intensity = 0.0;
        maxDist = (double)((hw*hw) + (hh*hh));  /* for 1x1 -> 0.0 */

        for (row = 0; row < rpng2_info.height; ++row) {
            y = row - hh;
            dest = bg_data + (size_t)row * bg_rowbytes;
            for (i = 0; i < rpng2_info.width; ++i) {
                x = i - hw;
                angle = (x == 0) ? PI_2 : atan((double)y / (double)x);
                gray = (double)MAX(ABS(y), ABS(x)) / grayspot;
                gray = MIN(1.0, gray);
                /* Vulnerable divide-by-zero: maxDist is 0.0 for a 1x1 image */
                dist = (double)((x*x) + (y*y)) / maxDist;  /* 0.0/0.0 on first pixel */
                intensity = cos((angle + (rotate * dist * PI)) * freq) * gray * saturate;
                intensity = (MAX(MIN(intensity, 1.0), -1.0) + 1.0) * 0.5;
                hue = (angle + PI) * INV_PI_360 + aoffset;
                s = gray * ((double)(ABS(x) + ABS(y)) / (double)(hw + hh));
                s = MIN(MAX(s, 0.0), 1.0);
                v = MIN(MAX(intensity, 0.0), 1.0);

                if (s == 0.0) {
                    ch = (uch)(v * 255.0);
                    *dest++ = ch;
                    *dest++ = ch;
                    *dest++ = ch;
                } else {
                    if ((hue < 0.0) || (hue >= 360.0))
                        hue -= (((int)(hue / 360.0)) * 360.0);
                    hue /= 60.0;
                    ii = (int)hue;
                    f = hue - (double)ii;
                    p = (1.0 - s) * v;
                    q = (1.0 - (s * f)) * v;
                    t = (1.0 - (s * (1.0 - f))) * v;
                    if      (ii == 0) { red = v; green = t; blue = p; }
                    else if (ii == 1) { red = q; green = v; blue = p; }
                    else if (ii == 2) { red = p; green = v; blue = t; }
                    else if (ii == 3) { red = p; green = q; blue = v; }
                    else if (ii == 4) { red = t; green = p; blue = v; }
                    else if (ii == 5) { red = v; green = p; blue = q; }
                    *dest++ = (uch)(red * 255.0);
                    *dest++ = (uch)(green * 255.0);
                    *dest++ = (uch)(blue * 255.0);
                }
            }
        }

        fprintf(stderr, "done.\n");
        fflush(stderr);
        free(bg_data);
    } else {
        fprintf(stderr, "Not the radial background path.\n");
    }
}

int main(void)
{
    /* Enable traps for floating-point divide-by-zero and invalid operations
       so that 0/0 or X/0 in double raises SIGFPE at the vulnerable line. */
#ifdef FE_DIVBYZERO
    feenableexcept(FE_DIVBYZERO);
#endif
#ifdef FE_INVALID
    feenableexcept(FE_INVALID);
#endif

    /* Craft a 1x1 image so hw = hh = 0 -> maxDist = 0 */
    rpng2_info.width = 1;
    rpng2_info.height = 1;

    /* Configure background type to radial (type & 0x07 == 2) */
    bg[0].type = 2;
    bg[0].bg_gray = 1;
    bg[0].bg_freq = 5;
    bg[0].bg_bsat = 10;
    bg[0].bg_brot = 0;

    rpng2_win_load_bg_image();
    return 0;
}
