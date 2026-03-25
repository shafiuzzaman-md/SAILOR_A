/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for jcprepct.c vulnerability */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Local typedefs/macros to satisfy the vulnerable statement without heavy includes */
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
#ifndef SIZEOF
#define SIZEOF(t) sizeof(t)
#endif
#ifndef MEMCOPY
#define MEMCOPY(dst,src,len) memcpy((dst),(src),(len))
#endif

typedef int boolean;
struct j_compress_struct { int dummy; };
typedef struct j_compress_struct * j_compress_ptr;

/* ENTRY: must be pure pass-through (no guards) */
