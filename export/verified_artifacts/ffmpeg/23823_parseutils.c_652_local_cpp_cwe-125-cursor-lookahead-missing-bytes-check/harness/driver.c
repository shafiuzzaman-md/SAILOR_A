#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

int entry_func(int64_t *timeval, const char *timestr, int duration);

int main() {
    // Concrete allocation for timeval
    int64_t tv = 0;

    // Concrete buffer for timestr content
    const size_t SZ = 16;
    char *buf = (char*)malloc(SZ);
    klee_make_symbolic(buf, SZ, "timestr_buf");

    // Place timestr at the END of the buffer so p[0] is out-of-bounds
    const char *timestr = buf + SZ;  // 0 bytes remaining

    // duration can be symbolic; it doesn't affect the dereference
    int duration;
    klee_make_symbolic(&duration, sizeof(duration), "duration");

    // Direct call to entry
    entry_func(&tv, timestr, duration);
    return 0;
}
