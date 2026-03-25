#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Neutralized vulnerable function (from parseutils.c around line 409)
// We keep only the minimal path to the vulnerable memcpy.
// CRITICAL: keep the vulnerable statement verbatim.
int av_parse_color(uint8_t *rgba_color, const char *color_string, int slen, void *log_ctx) {
    typedef struct { uint8_t rgb_color[3]; } ColorEntry;
    ColorEntry *entry = (ColorEntry*)malloc(sizeof(ColorEntry));
    if (!entry) return -1;
    klee_make_symbolic(entry, sizeof(*entry), "entry_rgb");

    // Vulnerable statement — copied verbatim
    memcpy(rgba_color, entry->rgb_color, 3);

    // Universal sink assertion (fires only if memcpy didn't crash)
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}

// ENTRY FUNCTION — MUST be a direct pass-through without guards
int entry_func(uint8_t *rgba_color, const char *color_string, int slen, void *log_ctx) {
    av_parse_color(rgba_color, color_string, slen, log_ctx);
    return 0;
}
