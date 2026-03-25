#include <stddef.h>
// Combined reproducer for 2998_bprint.c_158_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FFMIN (auto-detected external) */
int FFMIN() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: sink (auto-detected external) */
int sink() { return 0; }

/* PROACTIVE: values (auto-detected external) */
int values() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef BUF_SZ
#define BUF_SZ 16
#endif

// entry_func is defined in harness/bprint.c
int entry_func(AVBPrint *buf, char c, unsigned n);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate the AVBPrint struct concretely
    AVBPrint *buf = (AVBPrint *)calloc(1, sizeof(AVBPrint));

    // Backing storage for buf->str: concrete size to let KLEE detect OOB
    char *storage = (char *)malloc(BUF_SZ);
    // Do NOT make heap storage symbolic to avoid make_symbolic invalid pointer issues

    // Initialize struct fields
    buf->str = storage;
    buf->size = BUF_SZ;  // not used by stubs but keeps layout plausible

    // Make a local symbolic len, then assign into struct (avoid symbolic on heap field)
    unsigned sym_len;
    memcpy(&sym_len, fuzz_data + (0), sizeof(sym_len));
    
     // little room left so writes overflow easily
    buf->len = sym_len;

    // Symbolic fill character and count n
    char c;
    memcpy(&c, fuzz_data + (sizeof(sym_len)), sizeof(c));

    unsigned n;
    memcpy(&n, fuzz_data + (sizeof(sym_len) + sizeof(c)), sizeof(n));
    // Reasonable range; allow large enough to overflow
    
    

    // Call entry (pass-through to av_bprint_chars)
    entry_func(buf, c, n);
    return 0;
}
