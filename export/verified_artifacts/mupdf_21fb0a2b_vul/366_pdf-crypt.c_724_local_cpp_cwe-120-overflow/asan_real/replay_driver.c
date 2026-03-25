#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate context and document/crypt structures
    fz_context ctx_obj; // dummy
    fz_context *ctx = &ctx_obj;

    pdf_document *doc = (pdf_document *)calloc(1, sizeof(pdf_document));
    pdf_crypt *crypt = (pdf_crypt *)calloc(1, sizeof(pdf_crypt));

    // Allocate a SMALL owner key buffer so memcpy(userpass, crypt->o, 32) over-reads
    unsigned char *o = (unsigned char *)malloc(8);
    { static const unsigned char crypt_o_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(o, crypt_o_bytes_data, (8 < sizeof(crypt_o_bytes_data)) ? 8 : sizeof(crypt_o_bytes_data)); };

    crypt->o = o;
    crypt->r = 3;      // hit the kept (r==3 || r==4) case in the vul func
    crypt->length = 40; // arbitrary; not used in our neutralized slice

    doc->crypt = crypt;

    // Call entry; our harness entry directly calls the vulnerable function
    const char *pwd = "A"; // ignored by our neutralized entry
    pdf_authenticate_password(ctx, doc, pwd);

    return 0;
}
