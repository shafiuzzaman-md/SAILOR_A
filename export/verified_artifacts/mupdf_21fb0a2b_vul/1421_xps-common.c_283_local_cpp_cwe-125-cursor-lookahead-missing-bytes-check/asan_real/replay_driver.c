#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Concrete allocations for required types
    fz_context ctx_obj; memset(&ctx_obj, 0, sizeof(ctx_obj));
    xps_document doc_obj; memset(&doc_obj, 0, sizeof(doc_obj));
    fz_matrix ctm; memset(&ctm, 0, sizeof(ctm));
    fz_rect area; memset(&area, 0, sizeof(area));
    xps_resource dict_obj; memset(&dict_obj, 0, sizeof(dict_obj));
    fz_xml xml_obj; memset(&xml_obj, 0, sizeof(xml_obj));

    // Provide non-NULL attributes so entry does not early-return (pass-through anyway)
    char base_uri[16] = "base";
    char opacity_att[16];
    { static const unsigned char opacity_att_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(opacity_att, opacity_att_data, (sizeof(opacity_att) < sizeof(opacity_att_data)) ? sizeof(opacity_att) : sizeof(opacity_att_data)); };
    opacity_att[sizeof(opacity_att)-1] = '\0';

    // Set up a zero-sized samples buffer to trigger OOB at samples[0]
    // (KLEE treats accesses to size-0 allocations as out-of-bounds)
    doc_obj.small_samples = (float *)malloc(0);

    // Direct call to entry function
    xps_begin_opacity(&ctx_obj, &doc_obj, ctm, area, base_uri, &dict_obj, opacity_att, &xml_obj);
    return 0;
}
