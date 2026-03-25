/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

// Minimal local definition to allow compilation; the vulnerable read happens
// during argument evaluation of the INV_HAAR4 call below.
#ifndef INV_HAAR4
#define INV_HAAR4(a,b,c,d, o0,o1,o2,o3, t0,t1,t2,t3,t4) do { \
    (void)(a); (void)(b); (void)(c); (void)(d); \
    (void)(t0); (void)(t1); (void)(t2); (void)(t3); (void)(t4); \
    /* keep side-effects minimal; arguments are evaluated before this macro */ \
} while (0)
#endif

// Vulnerable function (kept verbatim around the sink)
void ff_ivi_col_haar4(const int32_t *in, int16_t *out, ptrdiff_t pitch,
