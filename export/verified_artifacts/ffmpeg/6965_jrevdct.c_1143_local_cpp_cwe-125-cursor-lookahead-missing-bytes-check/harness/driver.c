#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <klee/klee.h>

// entry_point is defined in harness/jrevdct.c
int entry_point(DCTBLOCK data);

int main() {
    // Allocate a small concrete buffer (2 bytes)
    unsigned char *raw = (unsigned char *)malloc(2);
    if (!raw) return 0;
    klee_make_symbolic(raw, 2, "raw_bytes");

    // Create a pointer past-the-end so a 2-byte load at data[0] is OOB
    DCTELEM *data = (DCTELEM *)(raw + 2);

    // Call pass-through entry to reach the vulnerable statement
    entry_point(data);
    return 0;
}
