#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry function from harness/context.c
int context_from_record(sepol_handle_t *h, const char *con, int version);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // 1) Concrete allocation of the handle object
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // 2) Concrete buffer for 'con' with symbolic contents
    const size_t CON_SZ = 64;
    char *con = (char *)malloc(CON_SZ);
    { memcpy(con, fuzz_data + 0, 64); };
    // ensure string is NUL-terminated to avoid library surprises
    con[CON_SZ - 1] = '\0';

    // 3) Version as a concrete small int (value not used by slice)
    int version = 0;

    // 4) Call entry — direct pass-through to vulnerable slice
    context_from_record(handle, con, version);

    return 0;
}
