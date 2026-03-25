#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <tiffio.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

typedef struct { int dummy; } TIFFOvrCache;

static int g_fail_next_tiffmalloc = 0;

/* Interpose libtiff's allocator: fail exactly once when instructed. */
void * _TIFFmalloc(tmsize_t s) {
    if (g_fail_next_tiffmalloc) {
        g_fail_next_tiffmalloc = 0; /* fail only once */
        return NULL;
    }
    return malloc((size_t)s);
}

void _TIFFfree(void *p) {
    free(p);
}

/* Stubs for functions used by the vulnerable code path. */
toff_t TIFF_WriteOverview(TIFF *hTIFF,
                          uint32_t nOXSize, uint32_t nOYSize,
                          int nBitsPerPixel, int nPlanarConfig, int nSamples,
                          uint32_t nOBlockXSize, uint32_t nOBlockYSize,
                          int bTiled, int nCompressFlag, int nPhotometric,
                          int nSampleFormat,
                          uint16_t *panRedMap, uint16_t *panGreenMap, uint16_t *panBlueMap,
                          int bUseSubIFDs, int nHorSubsampling, int nVerSubsampling)
{
    (void)hTIFF; (void)nOXSize; (void)nOYSize; (void)nBitsPerPixel; (void)nPlanarConfig; (void)nSamples;
    (void)nOBlockXSize; (void)nOBlockYSize; (void)bTiled; (void)nCompressFlag; (void)nPhotometric;
    (void)nSampleFormat; (void)panRedMap; (void)panGreenMap; (void)panBlueMap;
    (void)bUseSubIFDs; (void)nHorSubsampling; (void)nVerSubsampling;
    return (toff_t)0; /* dummy directory offset */
}

TIFFOvrCache * TIFFCreateOvrCache(TIFF *hTIFF, toff_t nDirOffset) {
    (void)hTIFF; (void)nDirOffset;
    /* Would normally create a cache. Return a non-NULL sentinel. */
    TIFFOvrCache *p = (TIFFOvrCache*)malloc(sizeof(TIFFOvrCache));
    if (p) p->dummy = 42;
    return p;
}

/* Minimal reproduction of the vulnerable function logic around line 864-892. */
int TIFFBuildOverviews(TIFF *hTIFF, int nOverviews, int *panOvList, const char *pszResampleMethod)
{
    (void)pszResampleMethod; /* unused in this minimal reproducer */

    /* Parameters set to simple constants to reach the vulnerable path. */
    uint16_t *panRedMap = NULL, *panGreenMap = NULL, *panBlueMap = NULL;
    uint32_t nXSize = 32, nYSize = 32;
    uint32_t nBlockXSize = 16, nBlockYSize = 16;
    int bTiled = 0; /* choose strips to avoid the tiled adjustment code */
    int nBitsPerPixel = 8;
    int nPlanarConfig = 1;
    int nSamples = 1;
    int nCompressFlag = 1;
    int nPhotometric = 1;
    int nSampleFormat = 1;
    int bUseSubIFDs = 0;
    int nHorSubsampling = 1, nVerSubsampling = 1;

    /* Vulnerable allocation: result not checked for NULL. */
    TIFFOvrCache **papoRawBIs = (TIFFOvrCache **)_TIFFmalloc((tmsize_t)(nOverviews * (tmsize_t)sizeof(void *)));

    for (int i = 0; i < nOverviews; i++)
    {
        uint32_t nOXSize, nOYSize, nOBlockXSize, nOBlockYSize;
        toff_t nDirOffset;

        nOXSize = (nXSize + (uint32_t)panOvList[i] - 1U) / (uint32_t)panOvList[i];
        nOYSize = (nYSize + (uint32_t)panOvList[i] - 1U) / (uint32_t)panOvList[i];

        nOBlockXSize = MIN(nBlockXSize, nOXSize);
        nOBlockYSize = MIN(nBlockYSize, nOYSize);

        nDirOffset = TIFF_WriteOverview(
            hTIFF, nOXSize, nOYSize, nBitsPerPixel, nPlanarConfig, nSamples,
            nOBlockXSize, nOBlockYSize, bTiled, nCompressFlag, nPhotometric,
            nSampleFormat, panRedMap, panGreenMap, panBlueMap, bUseSubIFDs,
            nHorSubsampling, nVerSubsampling);

        /* Null-pointer-dereference when papoRawBIs is NULL (due to failed allocation). */
        papoRawBIs[i] = TIFFCreateOvrCache(hTIFF, nDirOffset);
    }

    return 1;
}

int main(void)
{
    /* One overview level is enough. */
    int panOvList[1] = { 2 };

    /* Force the next _TIFFmalloc call (in TIFFBuildOverviews at line 864) to return NULL. */
    g_fail_next_tiffmalloc = 1;

    /* hTIFF is not used by our stubs; pass NULL. This will trigger the crash inside TIFFBuildOverviews. */
    (void)TIFFBuildOverviews(NULL, 1, panOvList, "nearest");

    /* We should never reach here due to the crash. */
    puts("If you see this, the bug did not trigger.");
    return 0;
}
