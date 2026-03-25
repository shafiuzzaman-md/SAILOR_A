#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// harness entry defined in harness/jsrun.c
extern int js_remove(js_State *J, int idx);

int main() {
    // Allocate state
    js_State *J = (js_State *)calloc(1, sizeof(js_State));

    // Tiny concrete stack to provoke OOB when TOP is large
    const int stack_elems = 2;
    js_Value *stk = (js_Value *)malloc(sizeof(js_Value) * stack_elems);
    // Make stack contents symbolic (addresses valid, bytes symbolic)
    { static const unsigned char stack_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(stk, stack_bytes_data, (sizeof(js_Value) * stack_elems < sizeof(stack_bytes_data)) ? sizeof(js_Value) * stack_elems : sizeof(stack_bytes_data)); };

    // Initialize J fields
    J->stack = stk;
    J->bot = 0;
    J->top = 5; // Larger than stack_elems to trigger OOB during shifting

    // idx symbolic but constrained to ensure the for-loop executes
    int idx;
    { static const unsigned char idx_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&idx, idx_data, (sizeof(idx) < sizeof(idx_data)) ? sizeof(idx) : sizeof(idx_data)); };
    // Normalize path to start at 0 after idx normalization and pass guard
    /* klee_assume removed */

    // Direct call to entry
    js_remove(J, idx);
    return 0;
}
