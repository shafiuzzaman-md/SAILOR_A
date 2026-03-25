/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* System-suggested local defs to avoid header churn */
#ifndef XML_HIDDEN
#define XML_HIDDEN  /* empty */
#endif
struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Minimal FreeType-ish typedefs/structs for this slice */
typedef int FT_Error;
typedef uint32_t FT_Encoding;
#define FT_Err_Ok 0
#define FT_THROW(x) (-1)
#define FT_ASSERT(x) ((void)0)

#define FT_ENCODING_UNICODE 0x00000001u
#define FT_ENCODING_NONE    0x00000000u

typedef struct FT_CharMapRec_ FT_CharMapRec_, *FT_CharMap;
typedef struct FT_FaceRec_ FT_FaceRec_, *FT_Face;

struct FT_CharMapRec_ {
    uint32_t encoding;
    uint16_t platform_id;
    uint16_t encoding_id;
};

struct FT_FaceRec_ {
    int num_charmaps;
    FT_CharMap *charmaps;   /* array of pointers to FT_CharMapRec_ */
    FT_CharMap charmap;     /* selected charmap */
};

/* Vulnerable helper from ftobjs.c — neutralized but keeps the vulnerable line verbatim */
