#include <string.h>
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// Prototype from harness
int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int main() {
    // Allocate a valid 8-byte output (Result)
    cmsUInt64Number result = 0;

    // Intentionally under-sized input buffer (6 bytes) to trigger pIn[6]
    uint8_t *in = (uint8_t*)malloc(6);
    if (!in) return 0;
    { static const unsigned char in_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(in, in_bytes_data, (6 < sizeof(in_bytes_data)) ? 6 : sizeof(in_bytes_data)); };

    // Call entry which directly invokes the vulnerable function
    _cmsAdjustEndianess64(&result, (cmsUInt64Number*)in);
    return 0;
}
