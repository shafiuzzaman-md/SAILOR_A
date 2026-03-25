#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Allocate the MQC context
    opj_mqc_t *mqc = (opj_mqc_t *)calloc(1, sizeof(opj_mqc_t));

    // Destination buffer: intentionally too small for OPJ_COMMON_CBLK_DATA_EXTRA (8) bytes
    // so that memcpy(mqc->end, mqc->backup, 8) overflows.
    unsigned char *dest = (unsigned char *)malloc(4);
    unsigned char *src  = (unsigned char *)malloc(8);

    // Make buffer contents symbolic (sizes are concrete)
    { static const unsigned char dest_buf_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(dest, dest_buf_data, (4 < sizeof(dest_buf_data)) ? 4 : sizeof(dest_buf_data)); };
    { static const unsigned char src_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, src_buf_data, (8 < sizeof(src_buf_data)) ? 8 : sizeof(src_buf_data)); };

    // Set pointers used by the vulnerable memcpy
    mqc->end = dest;     // only 4 bytes available at destination
    mqc->backup = src;   // 8 readable bytes available at source

    // Call entry which directly invokes the vulnerable function
    opq_mqc_finish_dec(mqc);
    return 0;
}
