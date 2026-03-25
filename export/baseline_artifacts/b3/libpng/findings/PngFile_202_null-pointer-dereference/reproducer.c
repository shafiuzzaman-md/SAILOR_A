#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Self-contained reproducer for the null-pointer-dereference in
 * contrib/visupng/PngFile.c::PngLoadImage when pBkgColor == NULL and
 * png_get_bKGD() returns true.
 */

typedef unsigned char byte;

/* Minimal libpng-like types used by the vulnerable code */
typedef struct png_struct_def* png_structp;
typedef struct png_info_def* png_infop;

typedef struct png_color_16_s {
    uint16_t index;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t gray;
} png_color_16;

typedef png_color_16* png_color_16p;

/* Macro used in the vulnerable call */
#define PNG_BACKGROUND_GAMMA_FILE 0

/* Stub implementations to force the vulnerable path */
int png_get_bKGD(png_structp png_ptr, png_infop info_ptr, png_color_16p* pBackground) {
    static png_color_16 bg;
    (void)png_ptr; (void)info_ptr;
    bg.red = 0x12;
    bg.green = 0x34;
    bg.blue = 0x56;
    *pBackground = &bg;  /* simulate that the PNG has a bKGD chunk */
    return 1;             /* indicate that bKGD is present */
}

void png_set_background(png_structp png_ptr, png_color_16p background, int gamma_code, int need_expand, double background_gamma) {
    (void)png_ptr; (void)background; (void)gamma_code; (void)need_expand; (void)background_gamma;
    /* no-op stub */
}

/* Struct matching how pBkgColor is used in the code */
typedef struct {
    byte red;
    byte green;
    byte blue;
} BkgColor;

/* Minimal reproduction of the vulnerable portion of PngLoadImage */
int PngLoadImage(BkgColor* pBkgColor) {
    png_structp png_ptr = NULL;  /* unused in this reproducer */
    png_infop info_ptr = NULL;   /* unused in this reproducer */
    png_color_16p pBackground = NULL;

    /* set the background color to draw transparent and alpha images over */
    if (png_get_bKGD(png_ptr, info_ptr, &pBackground))
    {
        png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
        /* Vulnerable NULL dereference when pBkgColor == NULL */
        pBkgColor->red   = (byte)pBackground->red;   /* crash here */
        pBkgColor->green = (byte)pBackground->green;
        pBkgColor->blue  = (byte)pBackground->blue;
    }
    else
    {
        pBkgColor = NULL;
    }

    return 0;
}

int main(void) {
    /* Trigger: pass NULL pBkgColor while png_get_bKGD() reports a present bKGD */
    (void)PngLoadImage(NULL);
    /* We should never get here; if we do, something went wrong with the trigger */
    puts("Unexpectedly survived the NULL dereference");
    return 0;
}
