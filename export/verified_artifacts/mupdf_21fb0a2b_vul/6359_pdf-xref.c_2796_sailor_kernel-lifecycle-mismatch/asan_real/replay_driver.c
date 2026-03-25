#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

// Prototypes from harness
void pdf_delete_object(fz_context *ctx, pdf_document *doc, int num);

int main() {
    // Allocate context and document concretely
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    pdf_document *doc = (pdf_document *)calloc(1, sizeof(pdf_document));

    // Ensure allocations succeeded
    if (!ctx || !doc) return 0;

    // Force the RHS of the vulnerable condition to be evaluated
    int num;
    { static const unsigned char num_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&num, num_data, (sizeof(num) < sizeof(num_data)) ? sizeof(num) : sizeof(num_data)); };
    /* klee_assume removed */ // so (num <= 0) is false and RHS is evaluated

    // Set up doc so that dereferencing doc->local_xref crashes at the target line
    doc->local_xref = NULL; // Null to trigger null-deref on doc->local_xref->num_objects
    doc->local_xref_nesting = 1; // Value irrelevant due to neutralized guards

    // Call entry (direct pass-through to vulnerable function in harness)
    pdf_delete_object(ctx, doc, num);

    return 0;
}
