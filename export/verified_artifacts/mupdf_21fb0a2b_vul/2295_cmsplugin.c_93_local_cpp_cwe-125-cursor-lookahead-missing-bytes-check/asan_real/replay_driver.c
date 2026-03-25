#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int main() {
    // Allocate adequate output buffer (8 bytes) to avoid early OOB writes
    unsigned char *resbuf = (unsigned char*)malloc(8);
    { static const unsigned char resbuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(resbuf, resbuf_data, (8 < sizeof(resbuf_data)) ? 8 : sizeof(resbuf_data)); };
    cmsUInt64Number *Result = (cmsUInt64Number*)resbuf;

    // Allocate undersized input buffer (5 bytes) so pIn[5] is OOB
    unsigned char *qbuf = (unsigned char*)malloc(5);
    { static const unsigned char qbuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(qbuf, qbuf_data, (5 < sizeof(qbuf_data)) ? 5 : sizeof(qbuf_data)); };
    cmsUInt64Number *QWord = (cmsUInt64Number*)qbuf;

    // Direct pass-through to vulnerable function via entry_func
    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
