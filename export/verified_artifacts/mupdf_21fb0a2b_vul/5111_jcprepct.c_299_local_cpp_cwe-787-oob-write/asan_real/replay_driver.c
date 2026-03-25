#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

/* Prototypes from harness */
void jinit_c_prep_controller(j_compress_ptr cinfo, boolean need_full_buffer);

int main() {
    j_compress_ptr cinfo = (j_compress_ptr)calloc(1, sizeof(*cinfo));
    /* need_full_buffer can be symbolic; entry is neutralized and will call through */
    boolean need_full_buffer;
    { static const unsigned char need_full_buffer_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&need_full_buffer, need_full_buffer_data, (sizeof(need_full_buffer) < sizeof(need_full_buffer_data)) ? sizeof(need_full_buffer) : sizeof(need_full_buffer_data)); };

    jinit_c_prep_controller(cinfo, need_full_buffer);
    return 0;
}
