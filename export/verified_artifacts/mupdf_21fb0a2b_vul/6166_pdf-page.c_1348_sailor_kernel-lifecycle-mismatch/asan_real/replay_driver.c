#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int main(void)
{
    // Concrete allocations
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    pdf_document *doc = (pdf_document *)calloc(1, sizeof(pdf_document));
    fz_page *p1 = (fz_page *)calloc(1, sizeof(fz_page));
    fz_page *p2 = (fz_page *)calloc(1, sizeof(fz_page));

    // Link list head
    doc->super.open = p1;
    p1->next = p2;
    p2->next = NULL;

    // Force an invalid prev target: allocate 1 slot, then point 2 past it.
    fz_page **arr = (fz_page **)malloc(sizeof(fz_page *) * 1);
    fz_page **badprev = arr + 2;   // OOB pointer target
    p1->prev = badprev;            // non-NULL, will be dereferenced and written

    // Make branch condition true: page->number == at
    int start;
    { static const unsigned char start_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&start, start_data, (sizeof(start) < sizeof(start_data)) ? sizeof(start) : sizeof(start_data)); };
    int num1;
    { static const unsigned char num1_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&num1, num1_data, (sizeof(num1) < sizeof(num1_data)) ? sizeof(num1) : sizeof(num1_data)); };
    /* klee_assume removed */
    p1->number = num1;

    // Unconstrained other page
    int num2;
    { static const unsigned char num2_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&num2, num2_data, (sizeof(num2) < sizeof(num2_data)) ? sizeof(num2) : sizeof(num2_data)); };
    p2->number = num2;

    int end = start + 1; // unused by harnessed entry

    // Call entry (direct pass-through to vul func in harness)
    pdf_delete_page_range(ctx, doc, start, end);
    return 0;
}
