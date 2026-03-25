#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type re-declarations to avoid pulling internal libtiff headers */
typedef uint16_t ttag_t;
typedef size_t tmsize_t;

typedef enum {
    TIFF_SHORT = 3
} TIFFDataType;

typedef struct {
    short tdir_type;
    uint32_t tdir_offset;
} TIFFDirEntry;

/* Minimal TIFF object layout with only the field we need */
typedef struct {
    struct {
        int td_samplesperpixel;
    } tif_dir;
} TIFF;

/* Stubs / interposed symbols */
void* _TIFFmalloc(tmsize_t s) {
    (void)s;
    /* Force allocation failure to trigger the NULL dereference */
    return NULL;
}

void _TIFFfree(void* p) {
    (void)p; /* no-op */
}

/* Variadic stub that sets the output short value */
int TIFFGetField(TIFF* tif, ttag_t tag, ...) {
    (void)tif;
    (void)tag;
    va_list ap;
    va_start(ap, tag);
    uint16_t* pv = va_arg(ap, uint16_t*);
    if (pv) *pv = 123; /* arbitrary value */
    va_end(ap);
    return 1;
}

/* Stubbed writer; not reached due to crash earlier but needed to compile */
int TIFFWriteShortArray(TIFF* tif, TIFFDataType type, ttag_t tag, TIFFDirEntry* dir, int samples, uint16_t* w) {
    (void)tif; (void)type; (void)tag; (void)dir; (void)samples; (void)w;
    return 1;
}

#define NITEMS(x) (sizeof(x) / sizeof((x)[0]))

/* Vulnerable function copied/adapted from contrib/pds/tif_pdsdirwrite.c */
static int TIFFWritePerSampleShorts(TIFF *tif, ttag_t tag, TIFFDirEntry *dir)
{
    uint16_t buf[10], v;
    uint16_t *w = buf;
    int i, status, samples = tif->tif_dir.td_samplesperpixel;

    if (samples > (int)NITEMS(buf))
        w = (uint16_t *)_TIFFmalloc((tmsize_t)samples * sizeof(uint16_t));
    TIFFGetField(tif, tag, &v);
    for (i = 0; i < samples; i++)
        w[i] = v; /* NULL deref when _TIFFmalloc() returned NULL */
    status = TIFFWriteShortArray(tif, TIFF_SHORT, tag, dir, samples, w);
    if (w != buf)
        _TIFFfree((void *)w);
    return (status);
}

int main(void) {
    TIFF *tif = (TIFF*)calloc(1, sizeof(TIFF));
    if (!tif) {
        fprintf(stderr, "alloc TIFF failed\n");
        return 1;
    }

    /* Ensure samples > 10 to take the malloc() branch */
    tif->tif_dir.td_samplesperpixel = 11;

    TIFFDirEntry dir;
    memset(&dir, 0, sizeof(dir));

    /* Any tag value is fine for this reproducer */
    ttag_t tag = 0;

    /* This call will crash with a NULL pointer dereference on w[i] */
    int ret = TIFFWritePerSampleShorts(tif, tag, &dir);

    printf("Unexpectedly returned: %d\n", ret);
    free(tif);
    return 0;
}
