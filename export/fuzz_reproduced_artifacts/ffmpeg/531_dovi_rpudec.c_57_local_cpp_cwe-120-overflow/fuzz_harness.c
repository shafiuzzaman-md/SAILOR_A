#include <stddef.h>
// Combined reproducer for 531_dovi_rpudec.c_57_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: length (auto-detected external) */
int length() { return 0; }

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Decls from harness
int dovi_entry(DOVIContext *s, const uint8_t *rpu, size_t rpu_size, int err_recognition);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context concretely
    DOVIContext *ctx = (DOVIContext *)calloc(1, sizeof(DOVIContext));

    // Concrete input buffer allocation (do not use symbolic sizes)
    const size_t BUF_SZ = 64; // fixed concrete size
    uint8_t *buf = (uint8_t *)malloc(BUF_SZ);

    // Make buffer contents symbolic
    memcpy(buf, fuzz_data + (0), BUF_SZ);

    // Symbolic size within bounds of allocation, but larger than dst (32) to trigger overflow
    size_t rpu_size;
    memcpy(&rpu_size, fuzz_data + (BUF_SZ), sizeof(rpu_size));
    
    

    // err_recognition symbolic (not used in neutralized path)
    int err_recognition;
    memcpy(&err_recognition, fuzz_data + (BUF_SZ + sizeof(rpu_size)), sizeof(err_recognition));

    // Direct call into entry which pass-throughs to vulnerable function
    dovi_entry(ctx, buf, rpu_size, err_recognition);
    return 0;
}
