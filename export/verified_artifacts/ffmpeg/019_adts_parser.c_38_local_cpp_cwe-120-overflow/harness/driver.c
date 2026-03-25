#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Entry prototype from harness
int entry_func(const uint8_t *buf, uint32_t *samples, uint8_t *frames);

#ifndef AV_AAC_ADTS_HEADER_SIZE
#define AV_AAC_ADTS_HEADER_SIZE 7
#endif

int main() {
    // Allocate a deliberately too-small buffer to trigger memcpy OOB read
    size_t small_sz = 3; // smaller than AV_AAC_ADTS_HEADER_SIZE (7)
    uint8_t *buf = (uint8_t *)malloc(small_sz);
    if (!buf) return 0;
    klee_make_symbolic(buf, small_sz, "buf_bytes");

    uint32_t samples = 0;
    uint8_t frames = 0;

    // Direct call to entry (which calls the vulnerable function)
    entry_func(buf, &samples, &frames);
    return 0;
}
