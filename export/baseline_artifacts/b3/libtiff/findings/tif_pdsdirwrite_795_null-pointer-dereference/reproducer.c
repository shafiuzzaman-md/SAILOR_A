#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Minimal stand-ins for libtiff types */
typedef struct TIFF TIFF; /* opaque */
typedef uint16_t ttag_t;

typedef enum {
    TIFF_NOTYPE = 0,
    TIFF_BYTE   = 1,
    TIFF_ASCII  = 2,
    TIFF_SHORT  = 3,
    TIFF_LONG   = 4,
    TIFF_RATIONAL = 5,
    TIFF_SBYTE  = 6,
    TIFF_UNDEFINED = 7,
    TIFF_SSHORT = 8,
    TIFF_SLONG  = 9,
    TIFF_SRATIONAL = 10,
    TIFF_FLOAT  = 11,
    TIFF_DOUBLE = 12,
    TIFF_IFD    = 13
} TIFFDataType;

typedef struct {
    uint16_t tdir_tag;
    int16_t  tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset;
} TIFFDirEntry;

/* Stubs mimicking libtiff internals/public helpers */
static uint32_t TIFFDataWidth(TIFFDataType type)
{
    switch (type) {
        case TIFF_BYTE:
        case TIFF_SBYTE:
        case TIFF_UNDEFINED:
            return 1;
        case TIFF_SHORT:
        case TIFF_SSHORT:
            return 2;
        case TIFF_LONG:
        case TIFF_SLONG:
        case TIFF_IFD:
        case TIFF_FLOAT:
            return 4;
        case TIFF_RATIONAL:
        case TIFF_SRATIONAL:
        case TIFF_DOUBLE:
            return 8;
        default:
            return 0;
    }
}

/* Force allocation failure to deterministically hit the bug */
static void* _TIFFmalloc(size_t s)
{
    (void)s;
    return NULL; /* simulate OOM */
}

static void _TIFFfree(void* p)
{
    (void)p;
}

/* Writer stub; not reached before the crash */
static int TIFFWriteByteArray(TIFF *tif, TIFFDirEntry *dir, char *v)
{
    (void)tif; (void)dir; (void)v;
    return 1;
}

/* Vulnerable function (reduced to relevant parts) from contrib/pds/tif_pdsdirwrite.c */
static int TIFFWriteAnyArray(TIFF *tif, TIFFDataType type, ttag_t tag,
                             TIFFDirEntry *dir, uint32_t n, double *v)
{
    char buf[10 * sizeof(double)];
    char *w = buf;
    int i, status = 0;

    if (n * TIFFDataWidth(type) > sizeof buf)
        w = (char *)_TIFFmalloc(n * TIFFDataWidth(type));
    switch (type)
    {
        case TIFF_BYTE:
        {
            unsigned char *bp = (unsigned char *)w;
            for (i = 0; i < (int)n; i++)
                bp[i] = (unsigned char)v[i]; /* NULL deref when w == NULL */
            dir->tdir_tag = tag;
            dir->tdir_type = (short)type;
            dir->tdir_count = n;
            if (!TIFFWriteByteArray(tif, dir, (char *)bp))
                goto out;
        }
        break;
        default:
            break;
    }
    status = 1;
out:
    if (w != buf)
        _TIFFfree(w);
    return status;
}

int main(void)
{
    TIFF *tif = NULL; /* not dereferenced in this path */
    TIFFDirEntry dir;

    /* Choose n so that n * TIFFDataWidth(TIFF_BYTE) > sizeof(buf) to trigger _TIFFmalloc */
    uint32_t n = 1000; /* 1000 * 1 > 80 (10 * sizeof(double)) */
    double *v = (double *)malloc((size_t)n * sizeof(double));
    if (!v) {
        fprintf(stderr, "malloc failed for v\n");
        return 1;
    }
    for (uint32_t i = 0; i < n; i++) v[i] = (double)(i & 0xFF);

    /* _TIFFmalloc is stubbed to return NULL, so w remains NULL and bp[i] dereferences NULL */
    (void)TIFFWriteAnyArray(tif, TIFF_BYTE, (ttag_t)123, &dir, n, v);

    free(v);
    return 0;
}
