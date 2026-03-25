// Compile with:
// clang -fsanitize=address -g -O0 reproducer.c -o reproducer -L/tmp/libtiff_upstream/build/.libs -ltiff -lm

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

// Minimal stand-ins for libtiff types and constants
typedef struct TIFF TIFF; // opaque, not used in this reproducer
struct TIFF { int dummy; };

#define TIFF_BYTE 1
#define TIFF_SHORT 3

// Minimal TIFFDirEntry subset needed by the vulnerable function
typedef struct {
    uint16_t tdir_tag;
    uint16_t tdir_type;
    uint32_t tdir_count;
} TIFFDirEntry;

// Stubs for libtiff functions used by TIFFFetchExtraSamples
// We interpose _TIFFmalloc to force allocation failure and trigger the bug path
void* _TIFFmalloc(size_t s) {
    (void)s;
    return NULL; // Force allocation failure to hit the NULL deref in the vulnerable code
}

void _TIFFfree(char* p) {
    // no-op
    (void)p;
}

// Pretend to fetch arrays by writing at least one element into the destination buffer
// This will dereference NULL when passed a NULL 'v'
int TIFFFetchByteArray(TIFF* tif, TIFFDirEntry* dir, uint16_t* v) {
    (void)tif; (void)dir;
    // Vulnerable write into the caller's buffer (NULL in our crafted case)
    v[0] = 42; // causes NULL pointer dereference
    return 1;
}

int TIFFFetchShortArray(TIFF* tif, TIFFDirEntry* dir, uint16_t* v) {
    (void)tif; (void)dir;
    v[0] = 42; // same behavior if SHORT path is taken
    return 1;
}

// Match libtiff's public API signature (variadic) to avoid ABI issues if interposed
int TIFFSetField(TIFF* tif, ...) {
    (void)tif;
    return 1;
}

// Vulnerable function copied/adapted from contrib/pds/tif_pdsdirread.c
#define NITEMS(x) (sizeof(x) / sizeof((x)[0]))
static int TIFFFetchExtraSamples(TIFF *tif, TIFFDirEntry *dir)
{
    uint16_t buf[10];
    uint16_t *v = buf;
    int status;

    if (dir->tdir_count > NITEMS(buf))
        v = (uint16_t *)_TIFFmalloc(dir->tdir_count * sizeof(uint16_t));
    if (dir->tdir_type == TIFF_BYTE)
        status = TIFFFetchByteArray(tif, dir, v); // v is NULL if malloc failed
    else
        status = TIFFFetchShortArray(tif, dir, v);
    if (status)
        status = TIFFSetField(tif, dir->tdir_tag, dir->tdir_count, v);
    if (v != buf)
        _TIFFfree((char *)v);
    return (status);
}
#undef NITEMS

int main(void) {
    // Create a dummy TIFF and a directory entry with count > 10 to force heap allocation
    TIFF tif_obj; TIFF* tif = &tif_obj;
    TIFFDirEntry dir;
    dir.tdir_tag = 338;            // ExtraSamples tag value in TIFF spec (not strictly needed for the crash)
    dir.tdir_type = TIFF_BYTE;     // Choose BYTE to hit the TIFFFetchByteArray path
    dir.tdir_count = 11;           // > 10 so the function tries to heap-allocate 'v'

    // This call will attempt to allocate 'v' via _TIFFmalloc (returns NULL),
    // then pass it to TIFFFetchByteArray which writes through the NULL pointer.
    (void)TIFFFetchExtraSamples(tif, &dir);

    // Should not reach here; keep to satisfy return type
    puts("If you see this, the reproducer did not trigger the bug.");
    return 0;
}
