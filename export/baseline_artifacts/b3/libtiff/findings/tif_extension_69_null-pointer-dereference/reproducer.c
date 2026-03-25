#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

/* Some builds may not expose these in public headers; redeclare to be safe. */
extern void TIFFSetClientInfo(TIFF *tif, void *data, const char *name);
extern void *TIFFGetClientInfo(TIFF *tif, const char *name);

/* Simple in-memory file implementation for TIFFClientOpen("w"). */
typedef struct {
    unsigned char *buf;
    toff_t size;
    toff_t capacity;
    toff_t pos;
} MemFile;

static int ensure_cap(MemFile *mf, toff_t need) {
    if (need <= mf->capacity)
        return 1;
    toff_t newcap = mf->capacity ? mf->capacity : 1024;
    while (newcap < need) {
        toff_t next = newcap * 2;
        if (next <= newcap) break; /* overflow guard */
        newcap = next;
    }
    unsigned char *nb = (unsigned char *)realloc(mf->buf, (size_t)newcap);
    if (!nb)
        return 0;
    mf->buf = nb;
    mf->capacity = newcap;
    return 1;
}

static tmsize_t mem_read(thandle_t ctx, void *buf, tmsize_t size) {
    MemFile *mf = (MemFile *)ctx;
    if (mf->pos >= mf->size)
        return 0;
    toff_t remain = mf->size - mf->pos;
    tmsize_t avail = (tmsize_t)(remain < (toff_t)size ? remain : (toff_t)size);
    if (avail > 0)
        memcpy(buf, mf->buf + mf->pos, (size_t)avail);
    mf->pos += avail;
    return avail;
}

static tmsize_t mem_write(thandle_t ctx, void *buf, tmsize_t size) {
    MemFile *mf = (MemFile *)ctx;
    if (size < 0)
        return 0;
    toff_t end = mf->pos + (toff_t)size;
    if (!ensure_cap(mf, end))
        return 0;
    if (size > 0)
        memcpy(mf->buf + mf->pos, buf, (size_t)size);
    mf->pos = end;
    if (mf->pos > mf->size)
        mf->size = mf->pos;
    return size;
}

static toff_t mem_seek(thandle_t ctx, toff_t off, int whence) {
    MemFile *mf = (MemFile *)ctx;
    toff_t newpos;
    switch (whence) {
    case 0: newpos = off; break;         /* SEEK_SET */
    case 1: newpos = mf->pos + off; break; /* SEEK_CUR */
    case 2: newpos = mf->size + off; break;/* SEEK_END */
    default: return (toff_t)-1;
    }
    if (newpos < 0)
        return (toff_t)-1;
    mf->pos = newpos;
    return mf->pos;
}

static int mem_close(thandle_t ctx) {
    MemFile *mf = (MemFile *)ctx;
    free(mf->buf);
    free(mf);
    return 0;
}

static toff_t mem_size(thandle_t ctx) {
    MemFile *mf = (MemFile *)ctx;
    return mf->size;
}

static int mem_map(thandle_t ctx, void **pbase, toff_t *psize) {
    (void)ctx;
    *pbase = NULL;
    *psize = 0;
    return 0; /* mapping unsupported */
}

static void mem_unmap(thandle_t ctx, void *base, toff_t size) {
    (void)ctx; (void)base; (void)size;
}

int main(void) {
    MemFile *mf = (MemFile *)calloc(1, sizeof(MemFile));
    if (!mf) {
        fprintf(stderr, "OOM\n");
        return 1;
    }

    TIFF *tif = TIFFClientOpen("mem", "w", (thandle_t)mf,
                               mem_read, mem_write, mem_seek,
                               mem_close, mem_size, mem_map, mem_unmap);
    if (!tif) {
        fprintf(stderr, "Failed to create TIFF handle\n");
        mem_close((thandle_t)mf);
        return 1;
    }

    /* Create at least one client info link with a non-NULL name. */
    int dummy = 1234;
    TIFFSetClientInfo(tif, &dummy, "foo");

    /* Trigger the bug: pass NULL name so strcmp(psLink->name, name) dereferences NULL. */
    (void)TIFFGetClientInfo(tif, NULL);

    /* If not crashed (patched library), clean up. */
    TIFFClose(tif);
    fprintf(stderr, "No crash (library likely patched).\n");
    return 0;
}
