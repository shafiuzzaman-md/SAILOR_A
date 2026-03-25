#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type re-declarations to avoid including internal libtiff headers */

typedef uint16_t ttag_t;

typedef enum {
    TIFF_NOTYPE = 0,
    TIFF_BYTE = 1,
    TIFF_ASCII = 2,
    TIFF_SHORT = 3,
    TIFF_LONG = 4,
    TIFF_RATIONAL = 5,
    TIFF_SBYTE = 6,
    TIFF_UNDEFINED = 7,
    TIFF_SSHORT = 8,
    TIFF_SLONG = 9,
    TIFF_SRATIONAL = 10,
    TIFF_FLOAT = 11,
    TIFF_DOUBLE = 12
} TIFFDataType;

/* Match libtiff internal layout just enough for this reproducer */
struct TIFFDirectory {
    uint16_t td_samplesperpixel;
};

typedef struct TIFF {
    struct TIFFDirectory tif_dir;
} TIFF;

/* Directory entry placeholder */
typedef struct {
    uint16_t dummy;
} TIFFDirEntry;

/* NITEMS helper from the original source */
#define NITEMS(x) (sizeof(x) / sizeof((x)[0]))

/* tmsize_t approximation (libtiff uses a signed size type) */
typedef size_t tmsize_t;

/* Stub implementations to control behavior and avoid needing full libtiff internals. */

/* Force allocation failure to exercise the buggy path. */
void* _TIFFmalloc(tmsize_t s) {
    (void)s;
    return NULL; /* Simulate OOM */
}

void _TIFFfree(void* p) {
    (void)p; /* no-op */
}

/* Minimal stub: just set a value and return success. */
int TIFFGetField(TIFF* tif, ttag_t tag, double* v) {
    (void)tif;
    (void)tag;
    if (v) *v = 3.14159;
    return 1;
}

/* Minimal stub: pretend writing succeeded. */
int TIFFWriteAnyArray(TIFF* tif, TIFFDataType type, ttag_t tag, TIFFDirEntry* dir, int samples, void* data) {
    (void)tif; (void)type; (void)tag; (void)dir; (void)samples; (void)data;
    return 1;
}

/* Vulnerable function copied and adapted from contrib/pds/tif_pdsdirwrite.c */
static int TIFFWritePerSampleAnys(TIFF *tif, TIFFDataType type, ttag_t tag, TIFFDirEntry *dir)
{
    double buf[10], v;
    double *w = buf;
    int i, status;
    int samples = (int)tif->tif_dir.td_samplesperpixel;

    if (samples > (int)NITEMS(buf))
        w = (double *)_TIFFmalloc((tmsize_t)samples * sizeof(double));
    TIFFGetField(tif, tag, &v);
    for (i = 0; i < samples; i++)
        w[i] = v; /* NULL deref when _TIFFmalloc failed and samples > 10 */
    status = TIFFWriteAnyArray(tif, type, tag, dir, samples, w);
    if (w != buf)
        _TIFFfree(w);
    return status;
}

int main(void) {
    TIFF tif;
    memset(&tif, 0, sizeof(tif));

    /* Ensure samples > 10 to take the allocation path (which we force to fail). */
    tif.tif_dir.td_samplesperpixel = 11; /* any value > 10 triggers the malloc path */

    TIFFDirEntry dir;
    memset(&dir, 0, sizeof(dir));

    /* Any tag/type values are fine for this reproducer. */
    TIFFDataType type = TIFF_DOUBLE;
    ttag_t tag = 0;

    /* This call will dereference a NULL pointer at w[i] = v; */
    (void)TIFFWritePerSampleAnys(&tif, type, tag, &dir);

    /* Not reached */
    return 0;
}
