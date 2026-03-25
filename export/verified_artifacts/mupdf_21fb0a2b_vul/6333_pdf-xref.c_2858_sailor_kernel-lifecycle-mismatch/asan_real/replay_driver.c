#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* Prototypes from the harness slice */
void pdf_update_object(fz_context *ctx, pdf_document *doc, int num, pdf_obj *newobj);

int main() {
    /* Allocate concrete objects */
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    pdf_document *doc = (pdf_document *)calloc(1, sizeof(pdf_document));
    pdf_obj *newobj = (pdf_obj *)calloc(1, sizeof(pdf_obj));

    /* Initialize document to avoid early returns and reach the target loop */
    doc->local_xref = 0;              /* ensure not taken */
    doc->local_xref_nesting = 0;      /* ensure not taken */

    /* Ensure we have at least 2 sections so j starts at 1 and enters loop */
    doc->num_xref_sections = 2;
    doc->xref_sections = (pdf_xref *)calloc((size_t)doc->num_xref_sections, sizeof(pdf_xref));

    /* Initialize section 1 (the one used at the vulnerable line) */
    int num_objects = 32; /* concrete, reasonable size */
    doc->xref_sections[1].num_objects = num_objects;
    doc->xref_sections[1].subsec = NULL; /* not used by our slice */

    /* Choose object number symbolically but within range (>=1, < num_objects) */
    int num;
    { static const unsigned char num_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&num, num_data, (sizeof(num) < sizeof(num_data)) ? sizeof(num) : sizeof(num_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    /* UAF setup: Free the xref_sections array before use in pdf_delete_object */
    free(doc->xref_sections);
    /* Do NOT nullify the pointer to simulate stale pointer retained in doc */

    /* Call entry; our neutralized entry directly calls the vulnerable function */
    pdf_update_object(ctx, doc, num, newobj);

    return 0;
}
