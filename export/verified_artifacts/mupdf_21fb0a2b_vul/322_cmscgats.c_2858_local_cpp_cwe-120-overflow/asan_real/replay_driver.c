#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int cmsIT8GetPatchName(cmsContext ContextID, cmsHANDLE hIT8, int nPatch, char* buffer); // from harness

int main() {
    // Allocate cmsIT8 object
    struct cmsIT8 *it8 = (struct cmsIT8*)calloc(1, sizeof(struct cmsIT8));

    // Source data: allocate larger than MAXSTR and fill with non-NUL to force strncpy to copy MAXSTR-1 bytes
    size_t src_sz = (size_t)MAXSTR + 16;
    char *src = (char*)malloc(src_sz);
    memset(src, 'A', src_sz); // no NUL within first MAXSTR-1 bytes

    // Destination buffer: intentionally too small to trigger overflow when copying MAXSTR-1 bytes
    size_t dst_sz = 64; // smaller than MAXSTR-1
    char *dst = (char*)malloc(dst_sz);
    memset(dst, 0x42, dst_sz);

    // Hook up the source into the object used by GetData stub
    it8->user_data = src;

    // Context and parameters
    cmsContext ctx = (cmsContext)0; // unused in harness
    cmsHANDLE handle = (cmsHANDLE)it8;
    int nPatch;
    { static const unsigned char nPatch_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&nPatch, nPatch_data, (sizeof(nPatch) < sizeof(nPatch_data)) ? sizeof(nPatch) : sizeof(nPatch_data)); };

    // Call entry (direct pass-through to vulnerable function)
    cmsIT8GetPatchName(ctx, handle, nPatch, dst);

    return 0;
}
