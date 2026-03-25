#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototypes from harness
fz_path *fz_clone_path(fz_context *ctx, fz_path *path);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate context and path
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_path *path = (fz_path *)calloc(1, sizeof(fz_path));

    // Prepare a small source buffer for cmds (e.g., 8 bytes)
    size_t src_sz = 8;
    unsigned char *cmds = (unsigned char *)malloc(src_sz);
    if (!ctx || !path || !cmds) return 0;

    // Make contents symbolic so KLEE doesn't concretize behavior
    { memcpy(cmds, fuzz_data + 0, 8); };

    // Set fields used by the harnessed entry
    path->cmds = cmds;           // block
    path->cmd_cap = 1024;        // len (intentionally larger than src_sz)

    // Call into entry which directly calls clone_block
    (void) fz_clone_path(ctx, path);

    return 0;
}
