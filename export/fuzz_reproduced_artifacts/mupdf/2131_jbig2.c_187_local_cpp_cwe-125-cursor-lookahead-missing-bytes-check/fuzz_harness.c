#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// Prototype for the target function defined in harness/parser.c
int16_t jbig2_get_int16(const unsigned char *bptr);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1) return 0;
    // Allocate a 1-byte buffer so access to bptr[1] is out-of-bounds
    unsigned char *buf = (unsigned char *)malloc(1);
    if (!buf) return 0; // keep runtime happy; klee won't explore this path much

    // Make the single byte symbolic
    { memcpy(buf, fuzz_data + 0, 1); };

    // Call the vulnerable function (reads bptr[0] and bptr[1])
    volatile int16_t r = jbig2_get_int16(buf);
    (void)r;

    return 0;
}
