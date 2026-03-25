/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for cache.c vulnerability */
#include <stdbool.h>
#include <stdlib.h>

/* Minimal local type defs to satisfy field accesses */
typedef struct bfd_iovec { int dummy; } bfd_iovec;

typedef struct bfd {
    const struct bfd_iovec *iovec;  /* used at the vulnerability line */
    void *iostream;                  /* referenced by original function, but neutralized */
    unsigned int flags;              /* kept for completeness; not used here */
} bfd;

/* Provide the cache_iovec symbol as in the original file */
static const struct bfd_iovec cache_iovec = { 0 };

/* VULNERABLE FUNCTION: keep signature and the exact vulnerable statement; neutralize control flow.
   Place sink assertion immediately AFTER the vulnerable statement to probe reachability. */
