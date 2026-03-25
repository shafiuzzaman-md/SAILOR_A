#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Self-contained stubs and minimal types to mimic the vulnerable logic. */

typedef struct bfd { int dummy; } bfd;

typedef struct {
    int32_t address;
    void *howto;
    int32_t addend;
} internal_reloc;

/* External reloc entry bytes layout (simplified). */
struct external_reloc {
    uint8_t r_address[4];
    uint8_t r_index[3];
    uint8_t r_type[1];
    uint8_t r_addend[4];
};

/* Mock constants/macros similar to those used by the real code. */
#define RELOC_EXT_BITS_EXTERN_LITTLE 0x80
#define RELOC_EXT_BITS_TYPE_LITTLE   0x03
#define RELOC_EXT_BITS_TYPE_SH_LITTLE 0

#define N_ABS 2

/* Stubs for bfd error reporting used by the target code. */
static void _bfd_error_handler(const char *fmt, ...) {
    (void)fmt; /* ignore in this reproducer */
}
static void bfd_set_error(int err) { (void)err; }

typedef enum { bfd_error_wrong_format = 1 } bfd_error_type;

/* Dummy howto table. */
static int howto_table_ext_storage[3] = {0, 1, 2};
static void *howto_table_ext = (void *)howto_table_ext_storage;

/* Little-endian 32-bit signed load from a 4-byte array. */
static inline int32_t GET_SWORD(bfd *abfd, const uint8_t b[4]) {
    (void)abfd;
    uint32_t u = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return (int32_t)u;
}

/* Vulnerable function, adapted from the snippet. We pass in symcount and symbols
   explicitly so MOVE_ADDRESS can index them. */
static void swap_ext_reloc_in(bfd *abfd,
                              const struct external_reloc *bytes,
                              internal_reloc *cache_ptr,
                              unsigned long symcount,
                              uint32_t *symbols)
{
    unsigned int r_index;
    unsigned int r_type;
    int r_extern;

    (void)abfd;

    cache_ptr->address = GET_SWORD(abfd, bytes->r_address);

    r_index = (((unsigned int)bytes->r_index[2] << 16) |
               ((unsigned int)bytes->r_index[1] << 8) |
               bytes->r_index[0]);

    r_extern = (0 != (bytes->r_type[0] & RELOC_EXT_BITS_EXTERN_LITTLE));

    r_type = ((bytes->r_type[0] & RELOC_EXT_BITS_TYPE_LITTLE) >> RELOC_EXT_BITS_TYPE_SH_LITTLE);

    if (r_type > 2) {
        _bfd_error_handler("unsupported relocation type imported: %#x", r_type);
        bfd_set_error(bfd_error_wrong_format);
    }

    /* howto_table_ext + r_type - just to mimic original behavior */
    cache_ptr->howto = (char *)howto_table_ext + (sizeof(int) * r_type);

    if (r_extern && r_index > symcount) {
        _bfd_error_handler("bad relocation record imported: %u", r_index);
        bfd_set_error(bfd_error_wrong_format);
        r_extern = 0;
        r_index = N_ABS;
    }

    /* MOVE_ADDRESS macro equivalent. Critically, it uses r_extern and symbols[r_index],
       which will read out-of-bounds when r_index == symcount (off-by-one). */
#define MOVE_ADDRESS(X) do { \
        if (r_extern) { \
            /* The following read is out-of-bounds when r_index == symcount. */ \
            volatile uint32_t symval = symbols[r_index]; \
            cache_ptr->addend = (X) + (int32_t)symval; \
        } else { \
            cache_ptr->addend = (X); \
        } \
    } while (0)

    MOVE_ADDRESS(GET_SWORD(abfd, bytes->r_addend));

#undef MOVE_ADDRESS
}

int main(void) {
    /* Set up a symbol array with symcount = 1, so the only valid index is 0. */
    unsigned long symcount = 1;
    uint32_t *symbols = (uint32_t *)malloc(symcount * sizeof(uint32_t));
    if (!symbols) {
        perror("malloc");
        return 1;
    }
    symbols[0] = 0x11111111u;

    /* Craft an external reloc with r_extern set and r_index == symcount (off-by-one). */
    struct external_reloc bytes;
    memset(&bytes, 0, sizeof(bytes));

    /* r_address and r_addend can be any values. */
    bytes.r_address[0] = 0x34; bytes.r_address[1] = 0x12; bytes.r_address[2] = 0x00; bytes.r_address[3] = 0x00; /* 0x1234 */
    bytes.r_addend[0]  = 0x78; bytes.r_addend[1]  = 0x56; bytes.r_addend[2]  = 0x00; bytes.r_addend[3]  = 0x00; /* 0x5678 */

    /* Encode r_index == symcount == 1 into 3 bytes little-endian. */
    unsigned int bad_index = (unsigned int)symcount; /* off-by-one */
    bytes.r_index[0] = (uint8_t)(bad_index & 0xFF);
    bytes.r_index[1] = (uint8_t)((bad_index >> 8) & 0xFF);
    bytes.r_index[2] = (uint8_t)((bad_index >> 16) & 0xFF);

    /* Set extern bit and a supported type (e.g., 1). */
    bytes.r_type[0] = RELOC_EXT_BITS_EXTERN_LITTLE | 0x01; /* r_extern = 1, r_type = 1 */

    internal_reloc cache;
    memset(&cache, 0, sizeof(cache));

    bfd dummy_bfd = {0};

    /* This call will attempt to read symbols[r_index] where r_index == symcount, */
    /* triggering an ASan out-of-bounds read. */
    swap_ext_reloc_in(&dummy_bfd, &bytes, &cache, symcount, symbols);

    /* Prevent optimizing away by using cache values. */
    printf("address=%d addend=%d howto=%p\n", cache.address, cache.addend, cache.howto);

    free(symbols);
    return 0;
}
