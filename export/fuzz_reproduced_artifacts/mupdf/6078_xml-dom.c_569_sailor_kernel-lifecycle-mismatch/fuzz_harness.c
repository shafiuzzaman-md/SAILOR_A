#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* entry_func is provided by the harness */
int fz_dom_remove_attribute(fz_context *ctx, fz_xml *elt, const char *att);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 2) return 0;
    // Allocate concrete context and xml node
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_xml *elt = (fz_xml *)calloc(1, sizeof(fz_xml));

    // Build a small attribute list: attr1 -> attr2 -> NULL
    struct attribute *attr1 = (struct attribute *)calloc(1, sizeof(struct attribute));
    struct attribute *attr2 = (struct attribute *)calloc(1, sizeof(struct attribute));

    // Concrete names for attributes so strcmp can run
    char *name1 = (char *)malloc(2); name1[0] = 'X'; name1[1] = '\0';
    char *name2 = (char *)malloc(2); name2[0] = 'Y'; name2[1] = '\0';

    attr1->name = name1; attr1->value = NULL; attr1->next = attr2;
    attr2->name = name2; attr2->value = NULL; attr2->next = NULL;

    // Place list on element
    elt->u.node.u.d.atts = attr1;

    // Create a UAF: free the head attribute struct, but keep elt->atts pointing to it
    free(attr1);

    // Symbolic attribute name to remove; any 1-char string is fine
    char att[2];
    { memcpy(att, fuzz_data + 0, 2); };
    /* klee_assume removed */
    // Optional: encourage traversal past first-branch equality (not required for UAF)
    /* klee_assume removed */

    // Call entry (simple pass-through to vulnerable function)
    fz_dom_remove_attribute(ctx, elt, att);
    return 0;
}
