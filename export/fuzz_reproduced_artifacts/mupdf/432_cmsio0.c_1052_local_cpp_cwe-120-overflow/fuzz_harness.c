#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate concrete _cmsICCPROFILE and set as hProfile
    _cmsICCPROFILE *icc = (_cmsICCPROFILE *)calloc(1, sizeof(*icc));
    if (!icc) return 0;
    cmsHPROFILE hProfile = (cmsHPROFILE)icc;

    // Allocate a too-small source buffer (8 bytes) to trigger over-read in memmove of 16 bytes
    cmsUInt8Number *src = (cmsUInt8Number *)malloc(8);
    if (!src) return 0;

    // Make the 8 bytes symbolic (content doesn't matter for overflow)
    { memcpy(src, fuzz_data + 0, 8); };

    // Context is unused in the vulnerable function; pass NULL
    cmsContext ctx = (cmsContext)0;

    // Direct call via entry_func to the vulnerable function
    cmsSetHeaderProfileID(ctx, hProfile, src);

    return 0;
}
