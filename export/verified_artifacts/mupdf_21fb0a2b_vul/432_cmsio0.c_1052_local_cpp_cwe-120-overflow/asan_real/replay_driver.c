#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Allocate concrete _cmsICCPROFILE and set as hProfile
    _cmsICCPROFILE *icc = (_cmsICCPROFILE *)calloc(1, sizeof(*icc));
    if (!icc) return 0;
    cmsHPROFILE hProfile = (cmsHPROFILE)icc;

    // Allocate a too-small source buffer (8 bytes) to trigger over-read in memmove of 16 bytes
    cmsUInt8Number *src = (cmsUInt8Number *)malloc(8);
    if (!src) return 0;

    // Make the 8 bytes symbolic (content doesn't matter for overflow)
    { static const unsigned char ProfileID_src_8_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, ProfileID_src_8_data, (8 < sizeof(ProfileID_src_8_data)) ? 8 : sizeof(ProfileID_src_8_data)); };

    // Context is unused in the vulnerable function; pass NULL
    cmsContext ctx = (cmsContext)0;

    // Direct call via entry_func to the vulnerable function
    cmsSetHeaderProfileID(ctx, hProfile, src);

    return 0;
}
