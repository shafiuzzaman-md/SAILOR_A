#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration of entry */
extern int adler32(unsigned long adler, const unsigned char *buf, unsigned int len);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    /* Make the target buffer intentionally SMALL to induce OOB in DO16(buf). */
    const size_t BUF_SIZE = 8; /* smaller than 16 so DO16 overreads */

    /* Allocate concrete buffer and make contents symbolic */
    unsigned char *buf = (unsigned char *)malloc(BUF_SIZE);
    if (!buf) return 0;
    { memcpy(buf, fuzz_data + 0, 8); };

    /* len symbolic: force path into while (len >= 16) and ensure len > BUF_SIZE */
    unsigned int len;
    { static const unsigned char len_data[] = {0x10, 0x00, 0x00, 0x00}; memcpy(&len, len_data, (sizeof(len) < sizeof(len_data)) ? sizeof(len) : sizeof(len_data)); };
    /* klee_assume removed */
    /* klee_assume removed */
    /* klee_assume removed */ /* cap to keep exploration reasonable */

    /* adler symbolic too */
    unsigned long adler;
    { static const unsigned char adler_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&adler, adler_data, (sizeof(adler) < sizeof(adler_data)) ? sizeof(adler) : sizeof(adler_data)); };

    adler32(adler, buf, len);
    return 0;
}
