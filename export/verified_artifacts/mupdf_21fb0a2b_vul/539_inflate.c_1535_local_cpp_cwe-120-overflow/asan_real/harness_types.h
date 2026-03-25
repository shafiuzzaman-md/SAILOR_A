/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness.c - neutralized slice for inflate.c:1535 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Local zlib-lite typedefs/macros to compile standalone */
#ifndef Z_OK
#define Z_OK 0
#endif
#ifndef Z_MEM_ERROR
#define Z_MEM_ERROR (-4)
#endif
#ifndef Z_NULL
#define Z_NULL NULL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef ENOUGH
#define ENOUGH 2048 /* reasonable local stand-in */
#endif
#ifndef voidpf
#define voidpf void*
#endif
#ifndef zmemcpy
#define zmemcpy memcpy
#endif

/* Minimal types to cover fields used in the slice */
typedef struct internal_state internal_state; /* forward */

typedef struct z_stream_s {
    internal_state *state; /* only field needed here */
} z_stream, *z_streamp;

struct inflate_state {
    /* Model code tables as int* for pointer arithmetic */
    int *codes;
    int *lencode;
    int *distcode;
    int *next;
    unsigned char *window;
    unsigned int wbits;
    z_streamp strm;
};

/* VULNERABLE FUNCTION (neutralized) */
