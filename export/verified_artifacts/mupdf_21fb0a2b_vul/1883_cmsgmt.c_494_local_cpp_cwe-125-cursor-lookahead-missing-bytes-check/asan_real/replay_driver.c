#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Entry defined in harness/cmsgmt.c
extern int cmsDetectTAC(cmsContext ContextID, cmsHPROFILE hProfile);

int main() {
    // Allocate simple concrete buffers for context and profile
    void *ctx = malloc(16);
    void *profile = malloc(16);

    // Make contents symbolic to overapproximate
    if (ctx) { static const unsigned char ctx_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(ctx, ctx_bytes_data, (16 < sizeof(ctx_bytes_data)) ? 16 : sizeof(ctx_bytes_data)); };
    if (profile) { static const unsigned char profile_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(profile, profile_bytes_data, (16 < sizeof(profile_bytes_data)) ? 16 : sizeof(profile_bytes_data)); };

    // Direct call into entry (no guards)
    cmsDetectTAC((cmsContext)ctx, (cmsHPROFILE)profile);
    return 0;
}
