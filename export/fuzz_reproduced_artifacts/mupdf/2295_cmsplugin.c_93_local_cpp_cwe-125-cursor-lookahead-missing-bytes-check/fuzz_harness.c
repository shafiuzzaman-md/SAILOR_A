#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 13) return 0;
    // Allocate adequate output buffer (8 bytes) to avoid early OOB writes
    unsigned char *resbuf = (unsigned char*)malloc(8);
    { memcpy(resbuf, fuzz_data + 0, 8); };
    cmsUInt64Number *Result = (cmsUInt64Number*)resbuf;

    // Allocate undersized input buffer (5 bytes) so pIn[5] is OOB
    unsigned char *qbuf = (unsigned char*)malloc(5);
    { memcpy(qbuf, fuzz_data + 8, 5); };
    cmsUInt64Number *QWord = (cmsUInt64Number*)qbuf;

    // Direct pass-through to vulnerable function via entry_func
    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
