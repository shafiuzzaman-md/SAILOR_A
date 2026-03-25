#include <stddef.h>
// Combined reproducer for 25624_mpegvideoencdsp.c_121_local_cpp_cwe-787-oob-write
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: draw_edges (auto-detected external) */
int draw_edges() { return 0; }

/* PROACTIVE: init (auto-detected external) */
int init() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Initialize context and function pointers
    MpegvideoEncDSPContext c = {0};
    ff_mpegvideoencdsp_init(&c, NULL);

    // Concrete dimensions (ensure buffer larger than MAX_LINE_SIZE)
    ptrdiff_t wrap = 64;   // bytes per line
    int height = 100;      // lines
    int width = 64;        // unused in impl but provided
    int w = 16;
    int h = 16;
    int sides = 0xF;       // arbitrary flags

    size_t buf_size = (size_t)(wrap * height); // exact size so write at index==size is OOB by 1
    uint8_t *buf = (uint8_t *)malloc(buf_size);
    if (!buf) return 0;

    // Make buffer contents symbolic for exploration
    memcpy(buf, fuzz_data + (0), buf_size);

    // Direct call to vulnerable path
    entry_func(&c, buf, wrap, width, height, w, h, sides);
    return 0;
}
