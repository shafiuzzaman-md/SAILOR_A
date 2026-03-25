// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Mirror minimal types from harness/context.c
typedef struct context_private_s {
    char *component[4];
    char *current_str;
} context_private_t;

typedef struct context_s {
    context_private_t *ptr;
} *context_t;

int context_str(context_t context);

int main() {
    // Allocate context and its private struct
    context_t ctx = (context_t)calloc(1, sizeof(*ctx));
    context_private_t *n = (context_private_t *)calloc(1, sizeof(*n));
    ctx->ptr = n;

    // Ensure conditional_free() path is exercised
    n->current_str = (char *)malloc(8);
    if (n->current_str) memset(n->current_str, '\0', 8);

    // Prepare components: at least component[1] non-NULL to hit inner loop and sink
    char *c0 = (char *)malloc(8);
    char *c1 = (char *)malloc(1);   // empty string triggers loop body safely

    // Make contents symbolic and NUL-terminated (size must match allocation)
    { static const unsigned char comp0_bytes_data[] = {0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; memcpy(c0, comp0_bytes_data, (8 < sizeof(comp0_bytes_data)) ? 8 : sizeof(comp0_bytes_data)); };
    c0[7] = '\0';
    c1[0] = '\0';

    n->component[0] = c0;
    n->component[1] = c1;  // non-NULL so the body executes and assertion fires
    n->component[2] = NULL;
    n->component[3] = NULL;

    // Call entry (directly calls context_str)
    context_str(ctx);
    return 0;
}
