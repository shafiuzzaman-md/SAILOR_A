#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// prototype from harness
int context_str(context_t ctx);

int main() {
    // Allocate context and its private struct
    context_t ctx = (context_t)calloc(1, sizeof(*ctx));
    context_private_t *n = (context_private_t *)calloc(1, sizeof(*n));

    // Allocate some component strings with concrete sizes and make them symbolic
    // Ensure at least component[0] is non-empty to exercise stpcpy path
    const size_t SZ = 16;
    char *c0 = (char *)malloc(SZ);
    char *c1 = (char *)malloc(SZ);
    char *c2 = (char *)malloc(SZ);
    char *c3 = (char *)malloc(SZ);

    { static const unsigned char comp0_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; memcpy(c0, comp0_data, (SZ < sizeof(comp0_data)) ? SZ : sizeof(comp0_data)); };
    { static const unsigned char comp1_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(c1, comp1_data, (SZ < sizeof(comp1_data)) ? SZ : sizeof(comp1_data)); };
    { static const unsigned char comp2_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(c2, comp2_data, (SZ < sizeof(comp2_data)) ? SZ : sizeof(comp2_data)); };
    { static const unsigned char comp3_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(c3, comp3_data, (SZ < sizeof(comp3_data)) ? SZ : sizeof(comp3_data)); };

    // Null-terminate to keep strlen/stpcpy well-defined when reached
    c0[SZ-1] = '\0';
    c1[SZ-1] = '\0';
    c2[SZ-1] = '\0';
    c3[SZ-1] = '\0';

    n->component[0] = c0;
    n->component[1] = c1;
    n->component[2] = c2;
    n->component[3] = c3;

    ctx->ptr = n;

    // Do NOT free n here; keep it alive so we can reach the sink in context_str

    // Call entry to trigger dereference of freed pointer in context_str
    context_str(ctx);
    return 0;
}
