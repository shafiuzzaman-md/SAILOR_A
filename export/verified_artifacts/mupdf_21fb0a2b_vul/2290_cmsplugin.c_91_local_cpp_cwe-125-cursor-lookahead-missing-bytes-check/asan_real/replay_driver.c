#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int main() {
    // Allocate output buffer: needs at least 5 bytes since pOut[4] is written
    unsigned char *out_buf = (unsigned char *)malloc(8);
    { static const unsigned char out_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(out_buf, out_buf_data, (8 < sizeof(out_buf_data)) ? 8 : sizeof(out_buf_data)); };

    // Allocate input buffer intentionally too small to trigger OOB read at pIn[3]
    unsigned char *in_buf = (unsigned char *)malloc(3);
    { static const unsigned char in_buf_data[] = {0x00, 0x00, 0x00}; memcpy(in_buf, in_buf_data, (3 < sizeof(in_buf_data)) ? 3 : sizeof(in_buf_data)); };

    // Cast to expected types
    cmsUInt64Number *Result = (cmsUInt64Number *)out_buf;
    cmsUInt64Number *QWord  = (cmsUInt64Number *)in_buf;

    // Direct call into entry function
    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
