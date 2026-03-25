#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype for entry function defined in harness/context.c
extern int context_new(const char *str);

int main() {
    // Small, fixed-size buffer WITHOUT forced NUL-termination
    char str[8];
    { static const unsigned char str_data[] = {0x3a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; memcpy(str, str_data, (sizeof(str) < sizeof(str_data)) ? sizeof(str) : sizeof(str_data)); };

    // Constrain first 8 bytes so the scanning loop keeps advancing and does not jump to err
    // Avoid NUL so the loop condition (*p) stays true inside the object, forcing it to walk past end
    for (int i = 0; i < (int)sizeof(str); ++i) {
        /* klee_assume removed */
        // Avoid characters that cause early goto err before we walk off the end
        /* klee_assume removed */
        /* klee_assume removed */
        /* klee_assume removed */
        /* klee_assume removed */
    }

    // Call entry (pass-through to context_new)
    context_new(str);
    return 0;
}
