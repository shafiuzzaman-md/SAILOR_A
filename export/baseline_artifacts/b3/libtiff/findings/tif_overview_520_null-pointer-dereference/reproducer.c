// Standalone reproducer for the NULL dereference in
// contrib/addtiffo/tif_overview.c:TIFF_DownSample_Subsampled at line ~520
// Build with:
//   clang -fsanitize=address -g -O0 -I/tmp/libtiff_upstream reproducer.c \
//         -L/tmp/libtiff_upstream/build/.libs -ltiff -lm -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// This is a minimal extraction of the vulnerable averaging branch in
// TIFF_DownSample_Subsampled, reduced to the parameters needed to reach the
// NULL-deref site. It mimics the exact pointer arithmetic around line 520.
static void TIFF_DownSample_Subsampled_repro(
    unsigned char* pabyOTile,
    unsigned char* pabySrcTile,             // This will be NULL to trigger the bug
    int nBlockXSize, int nBlockYSize,
    int nOBlockXSize, int nOBlockYSize,
    int nTXOff, int nTYOff,
    int nOMult,
    int nSample,
    int nHorSubsampling, int nVerSubsampling,
    int nSampleBlockSize,
    int nSourceSampleRowSize,
    int nDestSampleRowSize,
    const char* pszResampling)
{
    int nSourceY, nDestY;
    int nSourceX, nDestX;
    int nSourceXSecEnd, nSourceYSecEnd;
    int nSourceYSec, nSourceXSec;
    int nCummulator = 0;
    int nCummulatorCount;

    // Branch that leads to the vulnerable dereference at line ~520.
    if (strncmp(pszResampling, "averag", 6) == 0 ||
        strncmp(pszResampling, "AVERAG", 6) == 0)
    {
        if (nSample == 0)
        {
            for (nSourceY = 0, nDestY = nTYOff; nSourceY < nBlockYSize;
                 nSourceY += nOMult, nDestY++)
            {
                if (nDestY >= nOBlockYSize)
                    break;

                for (nSourceX = 0, nDestX = nTXOff; nSourceX < nBlockXSize;
                     nSourceX += nOMult, nDestX++)
                {
                    if (nDestX >= nOBlockXSize)
                        break;

                    nSourceXSecEnd = nSourceX + nOMult;
                    if (nSourceXSecEnd > nBlockXSize)
                        nSourceXSecEnd = nBlockXSize;
                    nSourceYSecEnd = nSourceY + nOMult;
                    if (nSourceYSecEnd > nBlockYSize)
                        nSourceYSecEnd = nBlockYSize;

                    nCummulator = 0;
                    for (nSourceYSec = nSourceY; nSourceYSec < nSourceYSecEnd; nSourceYSec++)
                    {
                        for (nSourceXSec = nSourceX; nSourceXSec < nSourceXSecEnd; nSourceXSec++)
                        {
                            // Vulnerable dereference when pabySrcTile is NULL
                            nCummulator += *(pabySrcTile +
                                             (nSourceYSec / nVerSubsampling) * nSourceSampleRowSize +
                                             (nSourceYSec % nVerSubsampling) * nHorSubsampling +
                                             (nSourceXSec / nHorSubsampling) * nSampleBlockSize +
                                             (nSourceXSec % nHorSubsampling));
                        }
                    }

                    nCummulatorCount = (nSourceXSecEnd - nSourceX) * (nSourceYSecEnd - nSourceY);

                    *(pabyOTile +
                      (nDestY / nVerSubsampling) * nDestSampleRowSize +
                      (nDestY % nVerSubsampling) * nHorSubsampling +
                      (nDestX / nHorSubsampling) * nSampleBlockSize +
                      (nDestX % nHorSubsampling)) =
                        (unsigned char)((nCummulator + (nCummulatorCount >> 1)) / nCummulatorCount);
                }
            }
        }
    }
}

int main(void)
{
    // Simulate the allocation failure in TIFFBuildOverviews: pabySrcTile == NULL
    unsigned char* pabySrcTile = NULL; // This triggers the NULL-deref in the loop above

    // Destination tile buffer: non-NULL so only source side faults
    size_t oTileSize = 16;
    unsigned char* pabyOTile = (unsigned char*)calloc(1, oTileSize);
    if (!pabyOTile) {
        fprintf(stderr, "Failed to allocate destination buffer\n");
        return 1;
    }

    // Parameters chosen to ensure we enter the averaging branch and execute
    // exactly one iteration that dereferences pabySrcTile.
    const char* pszResampling = "average"; // triggers the "averag" branch
    int nSample = 0;                // go into the nSample == 0 sub-branch
    int nOMult = 1;
    int nBlockXSize = 1;
    int nBlockYSize = 1;
    int nOBlockXSize = 1;
    int nOBlockYSize = 1;
    int nTXOff = 0;
    int nTYOff = 0;
    int nHorSubsampling = 1;
    int nVerSubsampling = 1;
    int nSampleBlockSize = 1;
    int nSourceSampleRowSize = 1;
    int nDestSampleRowSize = 1;

    // Call into the minimal clone of the vulnerable function.
    // The first read from pabySrcTile will be a NULL dereference at the
    // equivalent of contrib/addtiffo/tif_overview.c:520.
    TIFF_DownSample_Subsampled_repro(
        pabyOTile, pabySrcTile,
        nBlockXSize, nBlockYSize,
        nOBlockXSize, nOBlockYSize,
        nTXOff, nTYOff,
        nOMult,
        nSample,
        nHorSubsampling, nVerSubsampling,
        nSampleBlockSize,
        nSourceSampleRowSize,
        nDestSampleRowSize,
        pszResampling);

    // If execution reaches here (it shouldn't), clean up.
    free(pabyOTile);
    return 0;
}
