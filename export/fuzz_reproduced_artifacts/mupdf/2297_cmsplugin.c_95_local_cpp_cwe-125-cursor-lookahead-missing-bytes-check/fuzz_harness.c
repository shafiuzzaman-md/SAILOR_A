#include <stddef.h>
#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

extern int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 7) return 0;
    cmsUInt64Number *out64 = (cmsUInt64Number *)malloc(sizeof(cmsUInt64Number));
    if (!out64) return 0;

    unsigned char *inbuf = (unsigned char *)malloc(7); // intentionally 7 bytes
    if (!inbuf) return 0;
    { memcpy(inbuf, fuzz_data + 0, 7); };

    _cmsAdjustEndianess64((cmsUInt64Number *)out64, (cmsUInt64Number *)inbuf);
    return 0;
}
