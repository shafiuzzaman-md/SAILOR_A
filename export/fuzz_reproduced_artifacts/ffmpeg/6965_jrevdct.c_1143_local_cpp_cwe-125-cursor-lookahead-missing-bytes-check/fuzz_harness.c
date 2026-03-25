#include <stddef.h>
// Combined reproducer for 6965_jrevdct.c_1143_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// entry_point is defined in harness/jrevdct.c
int entry_point(DCTBLOCK data);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate a small concrete buffer (2 bytes)
    unsigned char *raw = (unsigned char *)malloc(2);
    if (!raw) return 0;
    memcpy(raw, fuzz_data + (0), 2);

    // Create a pointer past-the-end so a 2-byte load at data[0] is OOB
    DCTELEM *data = (DCTELEM *)(raw + 2);

    // Call pass-through entry to reach the vulnerable statement
    entry_point(data);
    return 0;
}
