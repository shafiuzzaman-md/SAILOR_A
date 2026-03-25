#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Minimal type definitions to avoid including libtiff headers */
typedef uint64_t toff_t;
typedef struct tiff TIFF; /* Opaque to us */

/* Fake values for TIFF tag constants used by the vulnerable function */
#define TIFFTAG_IMAGEWIDTH       256
#define TIFFTAG_IMAGELENGTH      257
#define TIFFTAG_BITSPERSAMPLE    258
#define TIFFTAG_SAMPLESPERPIXEL  277
#define TIFFTAG_PLANARCONFIG     284
#define TIFFTAG_ROWSPERSTRIP     278
#define TIFFTAG_TILEWIDTH        322
#define TIFFTAG_TILELENGTH       323

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Declarations for libtiff APIs referenced by the vulnerable function.
   They will be resolved from -ltiff at link time but never reached at runtime
   in this reproducer since we crash earlier. */
extern toff_t TIFFCurrentDirOffset(TIFF *);
extern int TIFFSetSubDirectory(TIFF *, toff_t);
extern int TIFFGetField(TIFF *, uint32_t, void *);
extern int TIFFIsTiled(TIFF *);
extern size_t TIFFStripSize(TIFF *);
extern size_t TIFFTileSize(TIFF *);

/* Overview cache structure as expected by the vulnerable function */
typedef struct
{
    toff_t nDirOffset;
    TIFF *hTIFF;
    uint32_t nXSize;
    uint32_t nYSize;
    uint16_t nBitsPerPixel;
    uint16_t nSamples;
    uint16_t nPlanarConfig;
    uint32_t nBlockYSize;
    uint32_t nBlockXSize;
    size_t nBytesPerBlock;
    int bTiled;
    uint32_t nBlocksPerRow;
    uint32_t nBlocksPerColumn;
} TIFFOvrCache;

/* Force allocation failure deterministically. We macro-substitute _TIFFmalloc
   used by the vulnerable function so we don't conflict with libtiff's symbol. */
static void *alloc_return_null(size_t s) {
    (void)s;
    return NULL;
}
#define _TIFFmalloc(sz) alloc_return_null((sz))

/* Vulnerable function (adapted from contrib/addtiffo/tif_ovrcache.c) */
TIFFOvrCache *TIFFCreateOvrCache(TIFF *hTIFF, toff_t nDirOffset)
{
    TIFFOvrCache *psCache;
    toff_t nBaseDirOffset;
    int nRet;

    psCache = (TIFFOvrCache *)_TIFFmalloc(sizeof(TIFFOvrCache));
    /* NULL pointer is dereferenced immediately without a NULL check */
    psCache->nDirOffset = nDirOffset;     /* <- crash here when psCache == NULL */
    psCache->hTIFF = hTIFF;

    /* The rest is kept to mirror the real function but will not be executed */
    nBaseDirOffset = TIFFCurrentDirOffset(psCache->hTIFF);
    nRet = TIFFSetSubDirectory(hTIFF, nDirOffset);
    (void)nRet;
    assert(nRet == 1);

    TIFFGetField(hTIFF, TIFFTAG_IMAGEWIDTH, &(psCache->nXSize));
    TIFFGetField(hTIFF, TIFFTAG_IMAGELENGTH, &(psCache->nYSize));

    TIFFGetField(hTIFF, TIFFTAG_BITSPERSAMPLE, &(psCache->nBitsPerPixel));
    TIFFGetField(hTIFF, TIFFTAG_SAMPLESPERPIXEL, &(psCache->nSamples));
    TIFFGetField(hTIFF, TIFFTAG_PLANARCONFIG, &(psCache->nPlanarConfig));

    if (!TIFFIsTiled(hTIFF))
    {
        TIFFGetField(hTIFF, TIFFTAG_ROWSPERSTRIP, &(psCache->nBlockYSize));
        psCache->nBlockXSize = psCache->nXSize;
        psCache->nBytesPerBlock = TIFFStripSize(hTIFF);
        psCache->bTiled = FALSE;
    }
    else
    {
        TIFFGetField(hTIFF, TIFFTAG_TILEWIDTH, &(psCache->nBlockXSize));
        TIFFGetField(hTIFF, TIFFTAG_TILELENGTH, &(psCache->nBlockYSize));
        psCache->nBytesPerBlock = TIFFTileSize(hTIFF);
        psCache->bTiled = TRUE;
    }

    psCache->nBlocksPerRow =
        (psCache->nXSize + psCache->nBlockXSize - 1) / psCache->nBlockXSize;
    psCache->nBlocksPerColumn =
        (psCache->nYSize + psCache->nBlockYSize - 1) / psCache->nBlockYSize;

    return psCache;
}

int main(void)
{
    /* Any TIFF* is fine; the crash happens before it's used. */
    TIFF *dummy = NULL;
    /* This call will trigger the NULL-dereference immediately. */
    TIFFCreateOvrCache(dummy, 0);

    /* Unreachable if the bug triggers correctly */
    puts("Bug did not trigger");
    return 0;
}
