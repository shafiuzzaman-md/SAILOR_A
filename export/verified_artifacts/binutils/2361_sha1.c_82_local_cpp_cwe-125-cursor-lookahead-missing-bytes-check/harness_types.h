/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for sha1_finish_ctx -> sha1_read_ctx */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Local definitions (avoid external headers) */
typedef uint32_t sha1_uint32;

#ifndef WORDS_BIGENDIAN
# define SWAP(n) \
    (((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
# define SWAP(n) (n)
#endif

/* Minimal struct definition satisfying accessed fields */
struct sha1_ctx {
    sha1_uint32 A, B, C, D, E;     /* state words used by sha1_read_ctx */
    sha1_uint32 total[2];          /* used by real finish, but not in neutralized path */
    sha1_uint32 buflen;            /* used by real finish, but not in neutralized path */
    sha1_uint32 buffer[32];        /* ensure at least 32 words if ever used */
};

/* Vulnerable function: keep exact vulnerable statement text and add sink assertion AFTER it */
void *
