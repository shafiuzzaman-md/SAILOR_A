#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

#ifndef JCS_YCCK
#define JCS_YCCK 7
#endif

int main() {
    // Allocate the decompressor struct
    struct jpeg_decompress_struct *cinfo = (struct jpeg_decompress_struct *)calloc(1, sizeof(struct jpeg_decompress_struct));
    if (!cinfo) return 0;

    // Set the target switch case symbolically as per driver_hint
    int sym_out_cs;
    { static const unsigned char out_color_space_data[] = {0x07, 0x00, 0x00, 0x00}; memcpy(&sym_out_cs, out_color_space_data, (sizeof(sym_out_cs) < sizeof(out_color_space_data)) ? sizeof(sym_out_cs) : sizeof(out_color_space_data)); };
    /* klee_assume removed */
    cinfo->out_color_space = sym_out_cs;

    // Provide a comp_info array that is too small (len=2):
    // comp_info[1] is valid, comp_info[2] is OOB and will trigger the bug
    cinfo->comp_info = (struct jpeg_component_info *)calloc(2, sizeof(struct jpeg_component_info));
    if (!cinfo->comp_info) return 0;

    // Call the entry/vulnerable function
    jinit_color_deconverter(cinfo);
    return 0;
}
