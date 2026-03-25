#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// entry from harness
int cmsFreeToneCurveTriple(cmsContext ctx, cmsToneCurve* CurveArr[3]);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate an undersized array deliberately: only 1 element
    cmsToneCurve **arr = (cmsToneCurve**)calloc(1, sizeof(cmsToneCurve*));

    // Make the single in-bounds element benign
    arr[0] = NULL;  // avoid free on a real object

    // Make the memory around the array symbolic to let KLEE explore Curve[1]
    // Note: we're not writing arr[1]; reading it should already trigger OOB
    { memcpy(arr, fuzz_data + 0, 8); };
    // Ensure arr[0] stays NULL so first branch doesn't free
    /* klee_assume removed */

    cmsContext ctx = (cmsContext)0;

    // Call the pass-through entry
    cmsFreeToneCurveTriple(ctx, (cmsToneCurve* (*)[3])arr);
    return 0;
}
