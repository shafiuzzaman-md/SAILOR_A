#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Self-contained stubs and typedefs mirroring the vulnerable code's expectations. */
typedef uint8_t bfd_size_type;   /* Intentionally narrow to force overflow in this reproducer. */
typedef unsigned char bfd_byte;

static void *bfd_zmalloc(bfd_size_type sz)
{
    void *p = malloc(sz);
    if (p)
        memset(p, 0, sz);
    return p;
}

/* Host-endian 32-bit store used by the vulnerable code. */
static void bfd_h_put_32(void *abfd, uint32_t val, void *where)
{
    (void)abfd; /* unused in this reproducer */
    memcpy(where, &val, 4);
}

/* Minimal struct placeholders from surrounding code (not functionally used here). */
struct internal_scnhdr {
    uint32_t s_size;
};

/* A reduced, self-contained version of xcoff_generate_rtinit that preserves the
   overflow-prone arithmetic and writes. */
static bool xcoff_generate_rtinit_like(const char *init, const char *fini)
{
    size_t initsz = init ? 1 + strlen(init) : 0;   /* matches 1 + strlen */
    size_t finisz = fini ? 1 + strlen(fini) : 0;   /* matches 1 + strlen */

    bfd_size_type data_buffer_size;
    bfd_byte *data_buffer;
    uint32_t val;

    /* Overflow-prone size computation done in bfd_size_type (intentionally uint8_t here). */
    data_buffer_size = (bfd_size_type)0x40;                    /* 0x40 base */
    data_buffer_size = (bfd_size_type)(data_buffer_size + (bfd_size_type)initsz);
    data_buffer_size = (bfd_size_type)(data_buffer_size + (bfd_size_type)finisz);
    data_buffer_size = (bfd_size_type)((data_buffer_size + 7) & ~(bfd_size_type)7);

    data_buffer = (bfd_byte *) bfd_zmalloc(data_buffer_size);
    if (data_buffer == NULL)
        return false;

    if (initsz) {
        /* These stores use fixed offsets (0x04 and 0x14). With the overflowed
           allocation, the second write will go out of bounds before memcpy. */
        val = 0x10;
        bfd_h_put_32(NULL, val, &data_buffer[0x04]);
        val = 0x40;
        bfd_h_put_32(NULL, val, &data_buffer[0x14]); /* OOB write when buffer is too small */
        memcpy(&data_buffer[val], init, initsz);     /* Would also overflow */
    }

    if (finisz) {
        val = 0x28;
        bfd_h_put_32(NULL, val, &data_buffer[0x08]);
        val = 0x40 + (uint32_t)initsz; /* as in the original */
        bfd_h_put_32(NULL, val, &data_buffer[0x2C]);
        memcpy(&data_buffer[val], fini, finisz);
    }

    /* Clean up (we won't get here if ASan aborts on the OOB). */
    free(data_buffer);
    return true;
}

int main(void)
{
    /* Craft inputs so that:
       initsz = 1 + strlen(init) = 200, finisz = 0
       data_buffer_size computed in uint8_t wraps:
       0x40 + 200 + 0 = 264 -> 264 % 256 = 8, aligned stays 8.
       Then writing 4 bytes at offset 0x14 (20) will overflow an 8-byte buffer. */
    size_t init_len = 199; /* so initsz = 200 */
    char *init = (char *)malloc(init_len + 1);
    if (!init) return 1;
    memset(init, 'A', init_len);
    init[init_len] = '\0';

    const char *fini = NULL; /* finisz = 0 to keep things minimal */

    /* This call should trigger an ASan heap-buffer-overflow. */
    bool ok = xcoff_generate_rtinit_like(init, fini);
    fprintf(stderr, "xcoff_generate_rtinit_like returned: %s\n", ok ? "true" : "false");

    free(init);
    return 0;
}
