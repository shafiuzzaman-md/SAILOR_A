#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Allocate concrete objects
    fz_context *ctx = (fz_context*)calloc(1, sizeof(fz_context));
    pdf_crypt *crypt = (pdf_crypt*)calloc(1, sizeof(pdf_crypt));
    pdf_obj *obj = (pdf_obj*)calloc(1, sizeof(pdf_obj));

    // Allocate a too-small buffer to trigger OOB read in memcpy(iv, s, 16)
    size_t small = 8; // less than 16
    unsigned char *buf = (unsigned char*)malloc(small);
    if (!ctx || !crypt || !obj || !buf) return 0;

    // Make buffer contents symbolic
    { static const unsigned char obj_str_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(buf, obj_str_buf_data, (small < sizeof(obj_str_buf_data)) ? small : sizeof(obj_str_buf_data)); };

    // Initialize pdf_obj fields
    obj->str = buf;
    obj->len = (int)small; // length shorter than 16

    // Call entry (strict pass-through harness calls vulnerable function)
    pdf_crypt_obj(ctx, crypt, obj, 0, 0);

    return 0;
}
