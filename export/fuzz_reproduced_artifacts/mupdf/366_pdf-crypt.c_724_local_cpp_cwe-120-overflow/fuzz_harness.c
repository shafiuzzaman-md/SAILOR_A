#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate context and document/crypt structures
    fz_context ctx_obj; // dummy
    fz_context *ctx = &ctx_obj;

    pdf_document *doc = (pdf_document *)calloc(1, sizeof(pdf_document));
    pdf_crypt *crypt = (pdf_crypt *)calloc(1, sizeof(pdf_crypt));

    // Allocate a SMALL owner key buffer so memcpy(userpass, crypt->o, 32) over-reads
    unsigned char *o = (unsigned char *)malloc(8);
    { memcpy(o, fuzz_data + 0, 8); };

    crypt->o = o;
    crypt->r = 3;      // hit the kept (r==3 || r==4) case in the vul func
    crypt->length = 40; // arbitrary; not used in our neutralized slice

    doc->crypt = crypt;

    // Call entry; our harness entry directly calls the vulnerable function
    const char *pwd = "A"; // ignored by our neutralized entry
    pdf_authenticate_password(ctx, doc, pwd);

    return 0;
}
