#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal stand-ins for libtiff types and tags */
typedef struct { int dummy; } TIFF;
#define TIFFTAG_ROWSPERSTRIP 278
#define TIFFTAG_TILEWIDTH    322
#define TIFFTAG_TILELENGTH   323
#define TIFFTAG_COLORMAP     320

/* Global fake colormap buffers (256 entries each, as for 8bpp palette) */
static uint16_t gRed[256];
static uint16_t gGreen[256];
static uint16_t gBlue[256];

/* Stubbed _TIFFmalloc that forces allocation failure to trigger the bug */
static void* _TIFFmalloc(size_t s) {
    (void)s;
    return NULL; /* Force failure so memcpy() will dereference NULL dest */
}

/* Stubbed warning handler setter used by the vulnerable function */
static void* TIFFSetWarningHandler(void* new_handler) {
    (void)new_handler;
    return NULL; /* return previous handler (unused here) */
}

/* Stubbed TIFFGetField that supplies the values the vulnerable code expects */
static int TIFFGetField(TIFF* tif, uint32_t tag, ...) {
    (void)tif;
    va_list ap;
    va_start(ap, tag);
    if (tag == TIFFTAG_ROWSPERSTRIP) {
        /* Return 0 so the code path assumes tiled image */
        uint32_t* p = va_arg(ap, uint32_t*);
        if (p) *p = 0;
        va_end(ap);
        return 0;
    } else if (tag == TIFFTAG_TILEWIDTH) {
        uint32_t* p = va_arg(ap, uint32_t*);
        if (p) *p = 16;
        va_end(ap);
        return 1;
    } else if (tag == TIFFTAG_TILELENGTH) {
        uint32_t* p = va_arg(ap, uint32_t*);
        if (p) *p = 16;
        va_end(ap);
        return 1;
    } else if (tag == TIFFTAG_COLORMAP) {
        /* Provide pointers to our global colormap buffers */
        uint16_t** pr = va_arg(ap, uint16_t**);
        uint16_t** pg = va_arg(ap, uint16_t**);
        uint16_t** pb = va_arg(ap, uint16_t**);
        if (pr) *pr = gRed;
        if (pg) *pg = gGreen;
        if (pb) *pb = gBlue;
        va_end(ap);
        return 1; /* Indicate a palette is present */
    }
    va_end(ap);
    return 0;
}

/* A minimal reproduction of the vulnerable portion of contrib/addtiffo/tif_overview.c */
static int TIFFBuildOverviews_repro(TIFF* hTIFF) {
    void* pfnWarning;
    uint32_t nBlockXSize = 0, nBlockYSize = 0;
    int bTiled = 0;
    uint32_t nXSize = 16, nYSize = 16; /* arbitrary */

    /* Turn off warnings (stub) */
    pfnWarning = TIFFSetWarningHandler(NULL);
    (void)pfnWarning;

    /* Decide between strips/tiles (force tiles via stub) */
    if (TIFFGetField(hTIFF, TIFFTAG_ROWSPERSTRIP, &nBlockYSize)) {
        nBlockXSize = nXSize;
        bTiled = 0;
    } else {
        TIFFGetField(hTIFF, TIFFTAG_TILEWIDTH, &nBlockXSize);
        TIFFGetField(hTIFF, TIFFTAG_TILELENGTH, &nBlockYSize);
        bTiled = 1;
    }
    (void)bTiled; /* not used further in this minimal repro */

    /* Capture the palette if there is one */
    uint16_t *panRedMap, *panGreenMap, *panBlueMap;
    if (TIFFGetField(hTIFF, TIFFTAG_COLORMAP, &panRedMap, &panGreenMap, &panBlueMap)) {
        uint16_t *panRed2, *panGreen2, *panBlue2;
        int nBitsPerPixel = 8; /* 8bpp palette => 256 colors */
        int nColorCount = 1 << nBitsPerPixel; /* 256 */

        /* Vulnerable allocations: results are not checked for NULL */
        panRed2   = (uint16_t*)_TIFFmalloc(2 * (size_t)nColorCount);
        panGreen2 = (uint16_t*)_TIFFmalloc(2 * (size_t)nColorCount);
        panBlue2  = (uint16_t*)_TIFFmalloc(2 * (size_t)nColorCount);

        /* NULL-pointer-dereference when allocation fails */
        memcpy(panRed2,   panRedMap,   2 * (size_t)nColorCount);
        memcpy(panGreen2, panGreenMap, 2 * (size_t)nColorCount);
        memcpy(panBlue2,  panBlueMap,  2 * (size_t)nColorCount);

        /* Not reached if crash occurs as expected */
        panRedMap = panRed2;
        panGreenMap = panGreen2;
        panBlueMap = panBlue2;
    } else {
        panRedMap = panGreenMap = panBlueMap = NULL;
    }

    return 0;
}

int main(void) {
    /* Initialize fake palette data (256 entries per channel) */
    for (int i = 0; i < 256; ++i) {
        gRed[i] = (uint16_t)i;
        gGreen[i] = (uint16_t)(255 - i);
        gBlue[i] = (uint16_t)((i * 2) & 0xFF);
    }

    TIFF fake;
    /* Call the function that contains the vulnerable logic */
    (void)TIFFBuildOverviews_repro(&fake);
    
    /* If the bug didn't trigger (it should), return non-zero */
    return 1;
}