// Combined reproducer for 3063_parseutils.c_409_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: assertion (auto-detected external) */
int assertion() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
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
