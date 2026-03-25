#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry declared in harness
int _cmsAdjustEndianess64(cmsUInt64Number* r, cmsUInt64Number* q);

int main() {
    // Allocate Result with at least 8 bytes so earlier writes don't crash before sink
    unsigned char *result_buf = (unsigned char*)malloc(8);
    if (!result_buf) return 0;
    { static const unsigned char result_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(result_buf, result_buf_data, (8 < sizeof(result_buf_data)) ? 8 : sizeof(result_buf_data)); };

    // Allocate QWord too small (2 bytes) so pIn[2] causes OOB read at the sink line
    unsigned char *qword_buf = (unsigned char*)malloc(2);
    if (!qword_buf) return 0;
    { static const unsigned char qword_buf_data[] = {0x00, 0x00}; memcpy(qword_buf, qword_buf_data, (2 < sizeof(qword_buf_data)) ? 2 : sizeof(qword_buf_data)); };

    // Call entry with casts to expected types
    cmsUInt64Number *Result = (cmsUInt64Number*)result_buf;
    cmsUInt64Number *QWord  = (cmsUInt64Number*)qword_buf;

    _cmsAdjustEndianess64(Result, QWord);
    return 0;
}
