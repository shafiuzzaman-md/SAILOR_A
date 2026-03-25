#include <stddef.h>
// Combined reproducer for 25622_mpegvideoencdsp.c_154_local_cpp_cwe-787-oob-write
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>

// Globals referenced by harness/spine (mpegvideoencdsp.c)
uint8_t *g_buf;
ptrdiff_t g_wrap;
int g_width, g_height, g_w, g_h, g_sides;

// Entry function prototype from harness
void ff_mpegvideoencdsp_init(MpegvideoEncDSPContext *c, void *avctx);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate a concrete buffer larger than MAX_LINE_SIZE (1920)
    size_t buf_size = 4096;  // concrete, fixed
    g_buf = (uint8_t *)malloc(buf_size);
    if (!g_buf) return 0;
    memcpy(g_buf, fuzz_data + (0), buf_size);

    // Make parameters symbolic to allow KLEE to explore OOB conditions
    memcpy(&g_wrap, fuzz_data + (buf_size), sizeof(g_wrap));
    memcpy(&g_width, fuzz_data + (buf_size + sizeof(g_wrap)), sizeof(g_width));
    memcpy(&g_height, fuzz_data + (buf_size + sizeof(g_wrap) + sizeof(g_width)), sizeof(g_height));
    memcpy(&g_w, fuzz_data + (buf_size + sizeof(g_wrap) + sizeof(g_width) + sizeof(g_height)), sizeof(g_w));
    memcpy(&g_h, fuzz_data + (buf_size + sizeof(g_wrap) + sizeof(g_width) + sizeof(g_height) + sizeof(g_w)), sizeof(g_h));
    memcpy(&g_sides, fuzz_data + (buf_size + sizeof(g_wrap) + sizeof(g_width) + sizeof(g_height) + sizeof(g_w) + sizeof(g_h)), sizeof(g_sides));

    // Basic sanity to avoid undefined behavior while keeping OOB reachable
    
    
    
    
    
    
    
    
    
    
    // g_sides unconstrained; not used in our spine

    // Call the entry function (directly triggers the vulnerable write inside)
    ff_mpegvideoencdsp_init(NULL, NULL);
    return 0;
}
