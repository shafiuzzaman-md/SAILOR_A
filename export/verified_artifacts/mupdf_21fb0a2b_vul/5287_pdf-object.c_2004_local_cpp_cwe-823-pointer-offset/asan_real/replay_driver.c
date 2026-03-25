// harness/driver.c
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

#ifndef ITEMS_CAP
#define ITEMS_CAP 8
#endif

int main() {
    // Allocate context
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));

    // Allocate a pdf_array object (as defined in harness_types.h from spine preamble)
    pdf_array *arr = (pdf_array *)calloc(1, sizeof(pdf_array));
    arr->super.kind = PDF_ARRAY; // Satisfy OBJ_IS_ARRAY

    // Allocate items array with a small fixed capacity
    arr->cap = ITEMS_CAP; // Not used in function, but set for completeness
    arr->items = (pdf_obj **)malloc(ITEMS_CAP * sizeof(pdf_obj *));
    // Initialize items to non-crashing values (NULLs are fine as pdf_drop_obj stub ignores)
    for (int k = 0; k < ITEMS_CAP; k++) arr->items[k] = 0;

    // Make len and i symbolic to allow KLEE to explore oversized moves
    int len_sym;
    { static const unsigned char len_sym_data[] = {0x00, 0x00, 0x00, 0x46}; memcpy(&len_sym, len_sym_data, (sizeof(len_sym) < sizeof(len_sym_data)) ? sizeof(len_sym) : sizeof(len_sym_data)); };
    /* klee_assume removed */            // Must be positive to pass delete path

    int i_sym;
    { static const unsigned char i_sym_data[] = {0x06, 0x00, 0x00, 0x00}; memcpy(&i_sym, i_sym_data, (sizeof(i_sym) < sizeof(i_sym_data)) ? sizeof(i_sym) : sizeof(i_sym_data)); };
    /* klee_assume removed */
    // Keep source/dest pointers within allocated array to start with
    /* klee_assume removed */
    // Ensure index in bounds w.r.t. len to avoid early throw
    /* klee_assume removed */

    // Assign into array struct
    arr->len = len_sym;

    // Call entry/vulnerable function directly
    pdf_array_delete(ctx, (pdf_obj *)arr, i_sym);

    return 0;
}
