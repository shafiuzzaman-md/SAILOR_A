#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Entry must be a direct pass-through to the vulnerable function
int av_parse_time(int64_t *timeval, const char *timestr, int duration);

int entry_func(int64_t *timeval, const char *timestr, int duration) {
    av_parse_time(timeval, timestr, duration);
    return 0;
}

// Neutralized vulnerable function keeping only the target path and the exact sink line
int av_parse_time(int64_t *timeval, const char *timestr, int duration) {
    const char *p = timestr;
    int negative = 0;

    /* parse timestr as a duration */
    if (p[0] == '-') {
        negative = 1;
        ++p;
    }

    // Universal sink assertion placed AFTER the vulnerable dereference
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}
