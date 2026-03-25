#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fenv.h>

/* Enable trapping of floating-point exceptions so 0/0 raises SIGFPE. */
/* feenableexcept is a GNU extension available on glibc systems. */
int feenableexcept(int);

/* Minimal re-declarations and helpers to reach the vulnerable code path */
typedef unsigned char uch;

#define ABS(a)   ((a) < 0 ? -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
/* Typical clip: clamp to [minv, maxv]. If maxv < minv, this returns minv. */
#define CLIP(x, minv, maxv) (((x) < (minv)) ? (minv) : (((x) > (maxv)) ? (maxv) : (x)))

#define PI 3.14159265358979323846
#define PI_2 (PI/2.0)
#define INV_PI_360 (180.0/PI)  /* radians -> degrees */
#define PROGNAME "repro"

/* Minimal globals mimicking the original */
struct { int width; int height; } rpng2_info;

typedef struct {
    int bg_gray;
    int bg_freq;
    int bg_bsat;
    int bg_brot;
} bgpat_t;

static bgpat_t bg[1];
static int pat = 0;

static uch *bg_data = NULL;
static int bg_rowbytes = 0;

/* Vulnerable function (reduced to the relevant region that computes 's') */
static void rpng2_win_load_bg_image(void)
{
    int row, i, x, y, ii;
    int hh, hw;
    uch ch;
    uch *dest;

    double freq, rotate, saturate, gray, intensity;
    double angle = 0.0, aoffset = 0.0, maxDist, dist;
    double red = 0.0, green = 0.0, blue = 0.0, hue, s, v, f, p, q, t;

    fprintf(stderr, "%s:  computing radial background...\n", PROGNAME);
    fflush(stderr);

    hh = rpng2_info.height / 2;
    hw = rpng2_info.width / 2;

    angle = CLIP(angle, 0.0, 360.0);
    /* NOTE: With a 1x1 image, (hh + hw) == 0; CLIP(..., 1, 0) yields 1 here. */
    int grayspot = CLIP(bg[pat].bg_gray, 1, (hh + hw));
    freq = MAX((double)bg[pat].bg_freq, 0.0);
    saturate = (double)bg[pat].bg_bsat * 0.1;
    rotate = (double)bg[pat].bg_brot * 0.1;
    gray = 0.0;
    intensity = 0.0;
    maxDist = (double)((hw*hw) + (hh*hh));

    for (row = 0; row < rpng2_info.height; ++row) {
        y = row - hh;
        dest = bg_data + row * bg_rowbytes;
        for (i = 0; i < rpng2_info.width; ++i) {
            x = i - hw;
            angle = (x == 0) ? PI_2 : atan((double)y / (double)x);
            /* gray computation is safe here because grayspot == 1 from CLIP above */
            gray = (double)MAX(ABS(y), ABS(x)) / (double)grayspot;
            gray = MIN(1.0, gray);
            dist = (maxDist != 0.0) ? (double)((x*x) + (y*y)) / maxDist : 0.0;
            intensity = cos((angle + (rotate * dist * PI)) * freq) * gray * saturate;
            intensity = (MAX(MIN(intensity, 1.0), -1.0) + 1.0) * 0.5;
            hue = (angle + PI) * INV_PI_360 + aoffset;
            /* Vulnerable divide: for a 1x1 image, (hw + hh) == 0, so denominator is 0. */
            s = gray * ((double)(ABS(x) + ABS(y)) / (double)(hw + hh));
            /* With x==0,y==0 on a 1x1 image, this is 0.0/0.0 -> FE_INVALID. */

            /* The code below is copied to keep the path realistic; it won't be reached
               if floating-point exceptions are enabled. */
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
}

int main(void)
{
    /* Trap both divide-by-zero and invalid (0/0) floating-point operations */
    feenableexcept(FE_DIVBYZERO | FE_INVALID);

    /* Craft a 1x1 image so that hw == 0 and hh == 0, making (hw + hh) == 0. */
    rpng2_info.width = 1;
    rpng2_info.height = 1;

    /* Background pattern parameters (values don't really matter here). */
    bg[0].bg_gray = 0;   /* With our CLIP macro, this becomes 1 and avoids earlier 0/0. */
    bg[0].bg_freq = 1;
    bg[0].bg_bsat = 5;
    bg[0].bg_brot = 0;

    bg_rowbytes = rpng2_info.width * 3; /* RGB */
    bg_data = (uch *)calloc((size_t)rpng2_info.height, (size_t)bg_rowbytes);
    if (!bg_data) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }

    /* Call the vulnerable routine; this will raise SIGFPE due to 0/0 at 's = ...'. */
    rpng2_win_load_bg_image();

    free(bg_data);
    return 0;
}
