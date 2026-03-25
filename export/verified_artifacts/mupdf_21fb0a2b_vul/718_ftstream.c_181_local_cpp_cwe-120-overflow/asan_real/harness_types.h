/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ftstream.c:181 overflow in FT_Stream_TryRead */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Minimal FreeType-like type defs */
#ifndef FT_TYPES_MINI
#define FT_TYPES_MINI
typedef unsigned char  FT_Byte;
typedef unsigned long  FT_ULong;
typedef long           FT_Long;

struct FT_StreamRec_;
typedef struct FT_StreamRec_* FT_Stream;

typedef FT_ULong (*FT_Stream_IoFunc)(FT_Stream stream,
                                     FT_ULong  offset,
                                     FT_Byte*  buffer,
                                     FT_ULong  count);

typedef struct FT_StreamRec_ {
    FT_Byte*         base;   /* memory base */
    FT_ULong         size;   /* stream size */
    FT_ULong         pos;    /* current position */
    FT_Stream_IoFunc read;   /* if non-NULL, custom read callback */
} FT_StreamRec;

#ifndef FT_MEM_COPY
#define FT_MEM_COPY(d,s,n) memcpy((d),(s),(n))
#endif
#endif /* FT_TYPES_MINI */

/* Vulnerable function (keep exact vulnerable statement) */
FT_ULong FT_Stream_TryRead( FT_Stream  stream,
                            FT_Byte*   buffer,
