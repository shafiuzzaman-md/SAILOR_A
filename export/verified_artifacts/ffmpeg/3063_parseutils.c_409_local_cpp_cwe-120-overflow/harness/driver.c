#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Forward decl from harness/parseutils.c
int entry_func(uint8_t *rgba_color, const char *color_string, int slen, void *log_ctx);

int main() {
    // Allocate an undersized buffer to trigger overflow on memcpy(..., ..., 3)
    uint8_t *rgba = (uint8_t *)malloc(2);  // intentionally too small
    if (!rgba) return 0;
    klee_make_symbolic(rgba, 2, "rgba_buf");

    // color_string and slen are unused by the harnessed function but provided for signature
    char *color_string = (char *)malloc(8);
    if (!color_string) return 0;
    klee_make_symbolic(color_string, 8, "color_string");
    // Ensure it's NUL-terminated to avoid any potential string routines if added later
    color_string[7] = '\0';

    int slen;
    klee_make_symbolic(&slen, sizeof(slen), "slen");

    void *log_ctx = NULL;

    // Direct call into entry function
    entry_func(rgba, color_string, slen, log_ctx);
    return 0;
}
