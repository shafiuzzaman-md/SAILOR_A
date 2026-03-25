#include <stddef.h>
// Combined reproducer for 019_adts_parser.c_38_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: AVERROR (auto-detected external) */
int AVERROR() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
// Entry prototype from harness
int entry_func(const uint8_t *buf, uint32_t *samples, uint8_t *frames);

#ifndef AV_AAC_ADTS_HEADER_SIZE
#define AV_AAC_ADTS_HEADER_SIZE 7
#endif

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate a deliberately too-small buffer to trigger memcpy OOB read
    size_t small_sz = 3; // smaller than AV_AAC_ADTS_HEADER_SIZE (7)
    uint8_t *buf = (uint8_t *)malloc(small_sz);
    if (!buf) return 0;
    memcpy(buf, fuzz_data + (0), small_sz);

    uint32_t samples = 0;
    uint8_t frames = 0;

    // Direct call to entry (which calls the vulnerable function)
    entry_func(buf, &samples, &frames);
    return 0;
}
