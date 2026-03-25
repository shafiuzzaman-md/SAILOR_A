#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Self-contained minimal stubs/types to exercise the vulnerable code path */
typedef long long toff_t;

#define COMPRESSION_NONE 1

typedef struct {
    uint16_t tiff_magic;
    uint16_t tiff_version;
} TIFFHeader;

typedef struct {
    uint16_t tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset; /* not used here */
} TIFFDirEntry;

typedef struct {
    uint32_t *td_stripbytecount;
    uint32_t *td_stripoffset;
    uint16_t td_nstrips;
    uint16_t td_compression;
    uint32_t td_samplesperpixel;
    uint32_t td_rowsperstrip;
    uint32_t td_imagelength;
} TIFFDirectory;

typedef struct {
    const char *tif_name;
    TIFFDirectory tif_dir;
} TIFF;

/* ----- Stubs mimicking libtiff internals/public helpers ----- */
static void _TIFFfree(void *p) { free(p); }

/* Force allocation failure to reproduce the NULL deref */
static void *CheckMalloc(TIFF *tif, size_t size, const char *what) {
    (void)tif; (void)size; (void)what;
    return NULL; /* Simulate OOM */
}

static uint32_t TIFFDataWidth(uint16_t t) {
    /* Minimal mapping: common TIFF types (not really used for the crash) */
    switch (t) {
        case 1: /* BYTE */ return 1;
        case 3: /* SHORT */ return 2;
        case 4: /* LONG */ return 4;
        case 5: /* RATIONAL */ return 8;
        default: return 1;
    }
}

static toff_t TIFFGetFileSize(TIFF *tif) {
    (void)tif;
    return 4096; /* arbitrary positive size */
}

static void TIFFSetFieldBit(TIFF *tif, int field) {
    (void)tif; (void)field;
}

static int TIFFFieldSet(TIFF *tif, int field) {
    (void)tif; (void)field;
    return 0;
}

static uint32_t TIFFScanlineSize(TIFF *tif) {
    (void)tif;
    return 0;
}

/* ----- Vulnerable function copied/simplified from contrib/pds/tif_pdsdirread.c ----- */
static void EstimateStripByteCounts(TIFF *tif, TIFFDirEntry *dir, uint16_t dircount)
{
    register TIFFDirEntry *dp;
    register TIFFDirectory *td = &tif->tif_dir;
    uint16_t i;

    if (td->td_stripbytecount)
        _TIFFfree(td->td_stripbytecount);
    td->td_stripbytecount =
        (uint32_t *)CheckMalloc(tif, td->td_nstrips * sizeof(uint32_t),
                                "for \"StripByteCounts\" array");
    if (td->td_compression != COMPRESSION_NONE)
    {
        uint32_t space =
            (uint32_t)(sizeof(TIFFHeader) + sizeof(uint16_t) +
                       (dircount * sizeof(TIFFDirEntry)) + sizeof(uint32_t));
        toff_t filesize = TIFFGetFileSize(tif);
        uint16_t n;

        /* calculate amount of space used by indirect values */
        for (dp = dir, n = dircount; n > 0; n--, dp++)
        {
            uint32_t cc = dp->tdir_count * TIFFDataWidth(dp->tdir_type);
            if (cc > sizeof(uint32_t))
                space += cc;
        }
        space = (uint32_t)((filesize - space) / td->td_samplesperpixel);
        for (i = 0; i < td->td_nstrips; i++)
            /* BUG: td_stripbytecount may be NULL if CheckMalloc fails */
            td->td_stripbytecount[i] = space; /* NULL dereference here */
        /* The rest is unreachable in our crash scenario */
        i--;
        if (td->td_stripoffset && td->td_stripbytecount &&
            td->td_stripoffset[i] + td->td_stripbytecount[i] > (uint32_t)filesize)
            td->td_stripbytecount[i] = (uint32_t)(filesize - td->td_stripoffset[i]);
    }
    else
    {
        uint32_t rowbytes = TIFFScanlineSize(tif);
        uint32_t rowsperstrip = td->td_imagelength / td->td_nstrips;
        for (i = 0; i < td->td_nstrips; i++)
            td->td_stripbytecount[i] = rowbytes * rowsperstrip;
    }
    TIFFSetFieldBit(tif, 0 /* FIELD_STRIPBYTECOUNTS placeholder */);
    if (!TIFFFieldSet(tif, 0 /* FIELD_ROWSPERSTRIP placeholder */))
        td->td_rowsperstrip = td->td_imagelength;
}

int main(void) {
    TIFF tif;
    memset(&tif, 0, sizeof(tif));
    tif.tif_name = "repro.tif";

    /* Initialize directory with values that take the compression!=NONE branch */
    tif.tif_dir.td_nstrips = 1;               /* ensure loop executes at least once */
    tif.tif_dir.td_compression = 2;           /* not COMPRESSION_NONE */
    tif.tif_dir.td_samplesperpixel = 1;       /* avoid div by zero */
    tif.tif_dir.td_imagelength = 10;

    /* Provide a dummy strip offset array (not needed for the crash, but harmless) */
    tif.tif_dir.td_stripoffset = (uint32_t*)malloc(sizeof(uint32_t));
    if (tif.tif_dir.td_stripoffset) tif.tif_dir.td_stripoffset[0] = 0;

    /* td_stripbytecount starts as NULL so _TIFFfree() is a no-op */
    tif.tif_dir.td_stripbytecount = NULL;

    /* Minimal directory entries array; contents don't matter for the crash */
    TIFFDirEntry dir[1];
    dir[0].tdir_type = 4;   /* LONG */
    dir[0].tdir_count = 1;  /* small */
    dir[0].tdir_offset = 0;

    /* This call will attempt to write into td_stripbytecount after a failed alloc */
    EstimateStripByteCounts(&tif, dir, 1);

    /* If the bug didn't trigger (it should), clean up */
    if (tif.tif_dir.td_stripoffset) free(tif.tif_dir.td_stripoffset);
    if (tif.tif_dir.td_stripbytecount) free(tif.tif_dir.td_stripbytecount);

    return 0;
}
