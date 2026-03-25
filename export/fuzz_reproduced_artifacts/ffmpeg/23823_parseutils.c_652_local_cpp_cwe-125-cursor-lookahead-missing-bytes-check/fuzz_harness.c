#include <stddef.h>
// Combined reproducer for 23823_parseutils.c_652_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>

int entry_func(int64_t *timeval, const char *timestr, int duration);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Concrete allocation for timeval
    int64_t tv = 0;

    // Concrete buffer for timestr content
    const size_t SZ = 16;
    char *buf = (char*)malloc(SZ);
    memcpy(buf, fuzz_data + (0), SZ);

    // Place timestr at the END of the buffer so p[0] is out-of-bounds
    const char *timestr = buf + SZ;  // 0 bytes remaining

    // duration can be symbolic; it doesn't affect the dereference
    int duration;
    memcpy(&duration, fuzz_data + (SZ), sizeof(duration));

    // Direct call to entry
    entry_func(&tv, timestr, duration);
    return 0;
}
