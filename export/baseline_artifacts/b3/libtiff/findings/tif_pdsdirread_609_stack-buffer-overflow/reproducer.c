#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal stand-ins for libtiff types/constants to keep this self-contained */

typedef struct { int dummy; } TIFF;

typedef struct {
    uint16_t tdir_tag;
    uint16_t tdir_type;
    uint32_t tdir_count;
    uint32_t tdir_offset;
} TIFFDirEntry;

/* TIFF data type constants (subset) */
#define TIFF_BYTE   1
#define TIFF_ASCII  2
#define TIFF_SHORT  3
#define TIFF_LONG   4
#define TIFF_SBYTE  6
#define TIFF_SSHORT 8

#if defined(__clang__) || defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

/* Stubs mimicking internal helpers. The key one is TIFFFetchByteArray() which
 * intentionally writes up to 4 elements, matching the fall-through behavior
 * described in the vulnerability.
 */
static int TIFFFetchShortArray(TIFF *tif, TIFFDirEntry *dir, uint16_t *v) {
    (void)tif;
    /* Safe path: only write up to 2 entries */
    for (uint32_t i = 0; i < dir->tdir_count && i < 2; i++) {
        v[i] = (uint16_t)(0x1000 + i);
    }
    return 1;
}

static int TIFFFetchByteArray(TIFF *tif, TIFFDirEntry *dir, uint16_t *v) {
    (void)tif;
    /* Vulnerable semantics: for count 3 or 4, write v[2] and v[3] due to
     * switch fall-through, even though caller only provided uint16_t v[2].
     */
    switch (dir->tdir_count) {
        case 4:
            v[3] = 0x4444; /* write past end of v[2] */
            /* fall through */
        case 3:
            v[2] = 0x3333; /* write past end of v[2] */
            /* fall through */
        case 2:
            v[1] = 0x2222;
            /* fall through */
        case 1:
            v[0] = 0x1111;
            break;
        default:
            /* For other counts, just claim success (not used here) */
            break;
    }
    return 1;
}

static void TIFFSetField(TIFF *tif, uint16_t tag, uint16_t a, uint16_t b) {
    (void)tif; (void)tag; (void)a; (void)b;
}

/* Vulnerable function replicated from contrib/pds/tif_pdsdirread.c */
static NOINLINE int TIFFFetchShortPair(TIFF *tif, TIFFDirEntry *dir) {
    uint16_t v[2];
    int ok = 0;

    switch (dir->tdir_type) {
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

int main(void) {
    TIFF tif = {0};
    TIFFDirEntry dir;

    /* Craft a directory entry with BYTE type and count=4 so that
     * TIFFFetchByteArray() writes 4 elements into v[2], overflowing the stack.
     */
    dir.tdir_tag = 273; /* arbitrary */
    dir.tdir_type = TIFF_BYTE; /* triggers the vulnerable branch */
    dir.tdir_count = 4;        /* will cause writes to v[0..3] */
    dir.tdir_offset = 0x11223344; /* arbitrary */

    /* This call should trigger an ASan stack-buffer-overflow report */
    int ok = TIFFFetchShortPair(&tif, &dir);

    printf("TIFFFetchShortPair returned %d (overflow should have been reported by ASan)\n", ok);
    return 0;
}
