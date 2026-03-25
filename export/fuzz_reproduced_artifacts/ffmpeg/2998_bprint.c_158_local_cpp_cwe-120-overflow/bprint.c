#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

// Minimal struct to support bprint operations
typedef struct AVBPrint {
    char *str;
    unsigned len;
    unsigned size;
} AVBPrint;

// Minimal prototypes used by av_bprint_chars
static unsigned av_bprint_room(const AVBPrint *buf);
static int av_bprint_alloc(AVBPrint *buf, unsigned extra_len);
static void av_bprint_grow(AVBPrint *buf, unsigned extra_len);

// Vulnerable function (neutralized, exact vulnerable statement preserved)
void av_bprint_chars(AVBPrint *buf, char c, unsigned n)
{
    unsigned room, real_n;

    while (1) {
        room = av_bprint_room(buf);
        if (n < room)
            break;
        if (av_bprint_alloc(buf, n))
            break;
    }
    if (room) {
        real_n = FFMIN(n, room - 1);
        memset(buf->str + buf->len, c, real_n);
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
    av_bprint_grow(buf, n);
}

// Entry function: strict pass-through to vulnerable function
int entry_func(AVBPrint *buf, char c, unsigned n) {
    av_bprint_chars(buf, c, n);
    return 0;
}

// Stubs — symbolic/neutral implementations
static unsigned av_bprint_room(const AVBPrint *buf) {
    // Over-approximate available room independently of actual allocation
    unsigned room;
    klee_make_symbolic(&room, sizeof(room), "av_bprint_room");
    // Ensure positive room so the vulnerable block is reachable
    klee_assume(room > 0);
    // Also keep within a reasonable bound to avoid explosion
    klee_assume(room <= 2048);
    return room;
}

static int av_bprint_alloc(AVBPrint *buf, unsigned extra_len) {
    // Neutralize allocation path: return non-zero to break the loop quickly
    // KLEE will still explore via room values
    (void)buf; (void)extra_len;
    int ret = 1;
    return ret;
}

static void av_bprint_grow(AVBPrint *buf, unsigned extra_len) {
    // Neutralized grow; keep state plausible but not necessary for triggering the sink
    (void)extra_len;
    (void)buf;
}
