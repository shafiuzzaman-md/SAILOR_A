#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int context_str(context_t context); // from harness

int main() {
    // Allocate the outer context object
    struct context_s *ctx = (struct context_s *)calloc(1, sizeof(struct context_s));
    if (!ctx) return 0;

    // Allocate the private payload
    context_private_t *n = (context_private_t *)calloc(1, sizeof(context_private_t));
    if (!n) return 0;
    ctx->ptr = n;

    // Prepare two components so the loop body executes (i == 1)
    const size_t s0 = 8, s1 = 8; // concrete sizes per model guidance
    char *c0 = (char *)malloc(s0);
    char *c1 = (char *)malloc(s1);
    if (!c0 || !c1) return 0;

    // Make contents symbolic; ensure null-termination within the buffers
    { static const unsigned char comp0_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(c0, comp0_data, (s0 < sizeof(comp0_data)) ? s0 : sizeof(comp0_data)); };
    { static const unsigned char comp1_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; memcpy(c1, comp1_data, (s1 < sizeof(comp1_data)) ? s1 : sizeof(comp1_data)); };
    c0[s0 - 1] = '\0';
    c1[s1 - 1] = '\0';

    // Set required components; others can be NULL
    n->component[0] = c0;       // required for first stpcpy
    n->component[1] = c1;       // ensures the loop body runs and hits target line 136
    n->component[2] = NULL;
    n->component[3] = NULL;

    // current_str starts NULL; conditional_free handles it safely
    n->current_str = NULL;

    // Call the entry which directly invokes the vulnerable function
    context_str((context_t)ctx);
    return 0;
}
