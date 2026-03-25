#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

extern int _cmsAdjustEndianess64(cmsUInt64Number* Result, cmsUInt64Number* QWord);

int main() {
    cmsUInt64Number *out64 = (cmsUInt64Number *)malloc(sizeof(cmsUInt64Number));
    if (!out64) return 0;

    unsigned char *inbuf = (unsigned char *)malloc(7); // intentionally 7 bytes
    if (!inbuf) return 0;
    { static const unsigned char inbuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(inbuf, inbuf_data, (7 < sizeof(inbuf_data)) ? 7 : sizeof(inbuf_data)); };

    _cmsAdjustEndianess64((cmsUInt64Number *)out64, (cmsUInt64Number *)inbuf);
    return 0;
}
