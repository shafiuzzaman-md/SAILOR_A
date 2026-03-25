#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal type re-declarations to match the vulnerable function's signature */
typedef uint16_t ttag_t;

typedef enum {
    TIFF_RATIONAL = 5,
    TIFF_SRATIONAL = 10
} TIFFDataType;

typedef struct {
    /* Only the members used by the vulnerable code */
    const char *tif_name;
} TIFF;

typedef struct {
    ttag_t tdir_tag;
    short tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset;
} TIFFDirEntry;

/* Stubs for internal/libtiff functions used by the vulnerable function */
static void * _TIFFmalloc(size_t size)
{
    /* Force allocation failure to trigger NULL dereference */
    (void)size;
    return NULL;
}

static void _TIFFfree(void *p)
{
    /* No-op (would normally call free) */
    (void)p;
}

static int TIFFWriteData(TIFF *tif, TIFFDirEntry *dir, char *data)
{
    /* Stubbed out, not reached due to crash */
    (void)tif; (void)dir; (void)data;
    return 1;
}

struct TIFFFieldFake { const char *field_name; };
static const struct TIFFFieldFake* _TIFFFieldWithTag(TIFF *tif, ttag_t tag)
{
    (void)tif; (void)tag;
    static const struct TIFFFieldFake f = { "fake_field" };
    return &f;
}

static void TIFFWarning(const char *name, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "TIFFWarning: %s: ", name ? name : "(null)");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

/* Vulnerable function copied/adapted from contrib/pds/tif_pdsdirwrite.c */
static int TIFFWriteRationalArray(TIFF *tif, TIFFDataType type, ttag_t tag,
                                  TIFFDirEntry *dir, uint32_t n, float *v)
{
    uint32_t i;
    uint32_t *t;
    int status;

    dir->tdir_tag = tag;
    dir->tdir_type = (short)type;
    dir->tdir_count = n;
    t = (uint32_t *)_TIFFmalloc(2 * n * sizeof(uint32_t));
    for (i = 0; i < n; i++)
    {
        float fv = v[i];
        int sign = 1;
        uint32_t den;

        if (fv < 0)
        {
            if (type == TIFF_RATIONAL)
            {
                TIFFWarning(tif->tif_name,
                            "%s: Information lost writing value (%g) as (unsigned) RATIONAL",
                            _TIFFFieldWithTag(tif, tag)->field_name, v[i]);
                fv = 0;
            }
            else
                fv = -fv, sign = -1;
        }
        den = 1L;
        if (fv > 0)
        {
            while (fv < 1L << (31 - 3) && den < 1L << (31 - 3))
                fv *= 1 << 3, den *= 1L << 3;
        }
        /* NULL pointer dereference when _TIFFmalloc() returns NULL */
        t[2 * i + 0] = sign * (fv + 0.5);
        t[2 * i + 1] = den;
    }
    status = TIFFWriteData(tif, dir, (char *)t);
    _TIFFfree((char *)t);
    return (status);
}

int main(void)
{
    TIFF tif;
    tif.tif_name = "repro.tif";

    TIFFDirEntry dir;
    float values[1] = { 1.2345f }; /* Positive to avoid warning path */

    /* n > 0 ensures the loop runs and dereferences t */
    (void)TIFFWriteRationalArray(&tif, TIFF_RATIONAL, (ttag_t)256, &dir, 1, values);

    return 0;
}
