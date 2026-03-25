#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal, self-contained reimplementation of the vulnerable code path
 * from contrib/pds/tif_pdsdirread.c to demonstrate the stack-buffer-overflow.
 */

typedef struct {
    struct {
        uint16_t tiff_magic;
    } tif_header;
} TIFF;

/* Minimal TIFFDirEntry struct with only the needed fields */
typedef struct {
    uint16_t tdir_tag;
    uint16_t tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset;
} TIFFDirEntry;

/* Endianness constants */
#define TIFF_BIGENDIAN    0x4d4d
#define TIFF_LITTLEENDIAN 0x4949

/* TIFF data type constants used here */
#define TIFF_BYTE   1
#define TIFF_SHORT  3
#define TIFF_LONG   4
#define TIFF_RATIONAL 5
#define TIFF_SBYTE  6
#define TIFF_SSHORT 8

/* Global buffer used by our TIFFFetchData stub to simulate file data */
static uint8_t* g_data = NULL;
static size_t g_data_len = 0;

static uint32_t TIFFDataWidth(uint16_t type)
{
    switch (type) {
        case TIFF_BYTE:
        case TIFF_SBYTE:    return 1;
        case TIFF_SHORT:
        case TIFF_SSHORT:   return 2;
        case TIFF_LONG:     return 4;
        case TIFF_RATIONAL: return 8;
        default:            return 1;
    }
}

/* Stub for TIFFSetField: keep it static to avoid clashing with libtiff's symbol */
static int TIFFSetField(TIFF* tif, uint16_t tag, uint16_t a, uint16_t b)
{
    (void)tif; (void)tag; (void)a; (void)b;
    /* Do nothing; presence ensures v[0], v[1] are read to prevent optimization. */
    return 1;
}

/* Stub for TIFFFetchData that copies dir->tdir_count * width bytes from g_data into buf. */
static int TIFFFetchData(TIFF* tif, TIFFDirEntry* dir, char* buf)
{
    (void)tif;
    size_t width = TIFFDataWidth(dir->tdir_type);
    size_t n = (size_t)dir->tdir_count * width;
    if (g_data && g_data_len >= n) {
        /* This memcpy is what will overflow the small destination buffer in the caller */
        memcpy(buf, g_data, n);
    } else {
        /* Still copy n bytes to trigger the overflow even if test data is short */
        for (size_t i = 0; i < n; i++) buf[i] = (char)(i & 0xFF);
    }
    return 1;
}

/* Unused in this reproducer, but declared to satisfy the call site */
static int TIFFFetchByteArray(TIFF *tif, TIFFDirEntry *dir, uint16_t *v)
{
    (void)tif; (void)dir; (void)v;
    return 0;
}

/* Vulnerable helper: identical logic to the one shown in the snippet */
static int TIFFFetchShortArray(TIFF *tif, TIFFDirEntry *dir, uint16_t *v)
{
    if (dir->tdir_count <= 2)
    {
        if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN)
        {
            switch (dir->tdir_count)
            {
                case 2:
                    v[1] = (uint16_t)(dir->tdir_offset & 0xffff);
                    /* fall through */
                case 1:
                    v[0] = (uint16_t)(dir->tdir_offset >> 16);
            }
        }
        else
        {
            switch (dir->tdir_count)
            {
                case 2:
                    v[1] = (uint16_t)(dir->tdir_offset >> 16);
                    /* fall through */
                case 1:
                    v[0] = (uint16_t)(dir->tdir_offset & 0xffff);
            }
        }
        return 1;
    }
    else {
        /* BUG: For count > 2, copies count shorts into v (only 2 elements) */
        return (TIFFFetchData(tif, dir, (char *)v) != 0);
    }
}

/* Vulnerable function from contrib/pds/tif_pdsdirread.c */
static int TIFFFetchShortPair(TIFF *tif, TIFFDirEntry *dir)
{
    uint16_t v[2];
    int ok = 0;

    switch (dir->tdir_type)
    {
        case TIFF_SHORT:
        case TIFF_SSHORT:
            ok = TIFFFetchShortArray(tif, dir, v);
            break;
        case TIFF_BYTE:
        case TIFF_SBYTE:
            ok = TIFFFetchByteArray(tif, dir, v);
            break;
    }
    if (ok)
        TIFFSetField(tif, dir->tdir_tag, v[0], v[1]);
    return ok;
}

int main(void)
{
    /* Prepare fake file data: 10 shorts = 20 bytes */
    uint32_t count = 10; /* > 2 to hit the overflow path */
    g_data_len = count * sizeof(uint16_t);
    g_data = (uint8_t*)malloc(g_data_len);
    if (!g_data) return 1;
    for (size_t i = 0; i < g_data_len; i++) g_data[i] = (uint8_t)(0xA0 + (i & 0x0F));

    TIFF tif;
    tif.tif_header.tiff_magic = TIFF_LITTLEENDIAN;

    TIFFDirEntry dir;
    dir.tdir_tag = 65000;      /* arbitrary */
    dir.tdir_type = TIFF_SHORT;/* triggers TIFFFetchShortArray path */
    dir.tdir_count = count;    /* count > 2 forces TIFFFetchData into 2-element v */
    dir.tdir_offset = 0;       /* unused by our TIFFFetchData stub */

    /* This call will cause TIFFFetchShortArray to memcpy 20 bytes into v[2] (4 bytes) */
    (void)TIFFFetchShortPair(&tif, &dir);

    free(g_data);
    return 0;
}
