#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Entry from harness
extern int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 12) return 0;
    // Allocate an 8-byte output buffer (safe writes)
    cmsUInt64Number *Result = (cmsUInt64Number*)malloc(sizeof(cmsUInt64Number)); // 8 bytes
    if (!Result) return 0;
    { memcpy(Result, fuzz_data + 0, 8); };

    // Allocate a TOO-SMALL input buffer (4 bytes) to trigger OOB read when accessing pIn[7]
    uint8_t *in4 = (uint8_t*)malloc(4); // 4 bytes only
    if (!in4) return 0;
    { memcpy(in4, fuzz_data + 8, 4); };

    // Cast the undersized buffer to cmsUInt64Number* (as in the vulnerable code)
    cmsUInt64Number *QWord = (cmsUInt64Number*)in4;

    // Direct call into the harness entry
    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
