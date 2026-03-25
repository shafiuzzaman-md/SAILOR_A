#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Prototype from harness
extern int pdf_dict_put_val_null(fz_context *ctx, pdf_obj *obj, int idx);

int main(void) {
    const int LEN = 4;

    pdf_obj_dict *dict = (pdf_obj_dict *)calloc(1, sizeof(pdf_obj_dict));
    if (!dict) return 0;
    dict->super.kind = 'd';
    dict->len = LEN;
    dict->items = (struct keyval *)calloc(LEN, sizeof(struct keyval));
    if (!dict->items) return 0;

    for (int i = 0; i < LEN; i++) {
        dict->items[i].k = NULL;
        dict->items[i].v = (pdf_obj *)malloc(sizeof(struct pdf_obj));
        if (dict->items[i].v) {
            dict->items[i].v->refs = 1;
            dict->items[i].v->kind = 'i';
            dict->items[i].v->flags = 0;
        }
    }

    int idx;
    { static const unsigned char idx_data[] = {0x02, 0x00, 0x00, 0x00}; memcpy(&idx, idx_data, (sizeof(idx) < sizeof(idx_data)) ? sizeof(idx) : sizeof(idx_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    // Pre-free the selected value to create a double-free when pdf_drop_obj is called
    free(dict->items[idx].v);
    // Keep the stale pointer value (do NOT null it) to model lifecycle mismatch

    pdf_dict_put_val_null(NULL, (pdf_obj *)dict, idx);
    return 0;
}
