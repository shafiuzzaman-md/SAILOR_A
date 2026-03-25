#include <klee/klee.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifndef AV_TS_MAX_STRING_SIZE
#define AV_TS_MAX_STRING_SIZE 1  // force edge case to make last become -1
#endif

#ifndef AV_NOPTS_VALUE
#define AV_NOPTS_VALUE (-9223372036854775807LL - 1)
#endif

#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif

// Minimal AVRational definition (FFmpeg-compatible signature)
typedef struct AVRational { int num; int den; } AVRational;

static inline double av_q2d(AVRational a) {
    if (a.den == 0) return 0.0;  // avoid div-by-zero
    return (double)a.num / (double)a.den;
}

// Vulnerable function copied from source_context (timestamp.c lines 21-36)
char *av_ts_make_time_string2(char *buf, int64_t ts, AVRational tb)
{
    if (ts == AV_NOPTS_VALUE) {
        snprintf(buf, AV_TS_MAX_STRING_SIZE, "NOPTS");
    } else {
        double val = av_q2d(tb) * ts;
        double log = (fpclassify(val) == FP_ZERO ? -INFINITY : floor(log10(fabs(val))));
        int precision = (isfinite(log) && log < 0) ? -log + 5 : 6;
        int last = snprintf(buf, AV_TS_MAX_STRING_SIZE, "%.*f", precision, val);
        last = FFMIN(last, AV_TS_MAX_STRING_SIZE - 1) - 1;
        for (; last && buf[last] == '0'; last--);
        for (; last && buf[last] != 'f' && (buf[last] < '0' || buf[0] > '9'); last--);
        klee_assert(0 && "SAILOR_SINK_REACHED");
        buf[last + 1] = '\0';
    }
    return buf;
}

// ENTRY: strict pass-through wrapper
int entry_func(char *buf, int64_t ts, AVRational tb) {
    av_ts_make_time_string2(buf, ts, tb);
    return 0;
}
