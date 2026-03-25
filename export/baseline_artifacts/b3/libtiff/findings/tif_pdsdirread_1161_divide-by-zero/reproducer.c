#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal re-declarations of libtiff-internal types used by the vulnerable function */
typedef size_t tsize_t;
typedef uint32_t tstrip_t;

typedef struct {
    uint32_t *td_stripbytecount;
    uint32_t *td_stripoffset;
    uint32_t td_rowsperstrip;
} TIFFDirectory;

typedef struct {
    TIFFDirectory tif_dir;
} TIFF;

/* Stubs for internal libtiff helpers referenced by the vulnerable code */
static tsize_t TIFFVTileSize(TIFF *tif, int dummy)
{
    (void)tif; (void)dummy;
    /* Crafted to trigger the bug: return 0 so rowbytes becomes 0 */
    return 0;
}

static tstrip_t TIFFhowmany(uint32_t x, tsize_t y)
{
    if (y == 0) return 0; /* defensive to avoid divide by zero here */
    return (tstrip_t)((x + (y - 1)) / y);
}

static void *_TIFFfree(void *p)
{
    free(p);
    return NULL;
}

static void *CheckMalloc(TIFF *tif, size_t n, const char *why)
{
    (void)tif; (void)why;
    return malloc(n);
}

/* Vulnerable function copied and minimally adapted from contrib/pds/tif_pdsdirread.c */
static void ChopUpSingleUncompressedStrip(TIFF *tif)
{
    TIFFDirectory *td = &tif->tif_dir;
    uint32_t bytecount = td->td_stripbytecount[0];
    uint32_t offset = td->td_stripoffset[0];
    tsize_t rowbytes = TIFFVTileSize(tif, 1), stripbytes;
    tstrip_t strip, nstrips, rowsperstrip;
    uint32_t *newcounts;
    uint32_t *newoffsets;

    /*
     * Make the rows hold at least one
     * scanline, but fill 8k if possible.
     */
    if (rowbytes > 8192)
    {
        stripbytes = rowbytes;
        rowsperstrip = 1;
    }
    else
    {
        /*
         * BUG: rowbytes can be 0 (from TIFFVTileSize). This triggers a
         * divide-by-zero here.
         */
        rowsperstrip = 8192 / rowbytes; /* divide-by-zero when rowbytes == 0 */
        stripbytes = rowbytes * rowsperstrip;
    }
    /* never increase the number of strips in an image */
    if (rowsperstrip >= td->td_rowsperstrip)
        return;
    nstrips = (tstrip_t)TIFFhowmany(bytecount, stripbytes);
    newcounts = (uint32_t *)CheckMalloc(tif, nstrips * sizeof(uint32_t),
                                        "for chopped \"StripByteCounts\" array");
    newoffsets = (uint32_t *)CheckMalloc(tif, nstrips * sizeof(uint32_t),
                                         "for chopped \"StripOffsets\" array");
    if (newcounts == NULL || newoffsets == NULL)
    {
        if (newcounts != NULL)
            _TIFFfree(newcounts);
        if (newoffsets != NULL)
            _TIFFfree(newoffsets);
        return;
    }

    for (strip = 0; strip < nstrips; strip++)
    {
        if (stripbytes > bytecount)
            stripbytes = bytecount;
        newcounts[strip] = stripbytes;
        newoffsets[strip] = offset;
        offset += stripbytes;
        bytecount -= stripbytes;
    }

    /* Normally would replace directory fields here; omitted as we never reach this with the bug. */
}

int main(void)
{
    /* Set up a minimal TIFF object with one strip to reach the vulnerable code */
    TIFF tif;
    memset(&tif, 0, sizeof(tif));

    /* Allocate and initialize required directory arrays */
    tif.tif_dir.td_stripbytecount = (uint32_t *)malloc(sizeof(uint32_t));
    tif.tif_dir.td_stripoffset = (uint32_t *)malloc(sizeof(uint32_t));
    if (!tif.tif_dir.td_stripbytecount || !tif.tif_dir.td_stripoffset)
    {
        fprintf(stderr, "Allocation failure\n");
        return 1;
    }

    /* Values are mostly irrelevant; we will crash before they're used meaningfully */
    tif.tif_dir.td_stripbytecount[0] = 1024; /* any non-zero */
    tif.tif_dir.td_stripoffset[0] = 0;       /* any value */
    tif.tif_dir.td_rowsperstrip = 0xFFFFFFFFu; /* large value to avoid early return if we passed the bug */

    /* Trigger the vulnerable path: TIFFVTileSize stub returns 0 making rowbytes = 0 */
    printf("About to trigger divide-by-zero in ChopUpSingleUncompressedStrip...\n");
    ChopUpSingleUncompressedStrip(&tif);

    /* Clean up (not reached on success) */
    free(tif.tif_dir.td_stripbytecount);
    free(tif.tif_dir.td_stripoffset);
    return 0;
}
