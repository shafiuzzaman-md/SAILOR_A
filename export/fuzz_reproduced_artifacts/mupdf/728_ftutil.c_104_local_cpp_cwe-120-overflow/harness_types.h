/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Minimal FreeType-like types and macros
typedef long FT_Long;
typedef int FT_Error;
typedef void* FT_Pointer;

#ifndef FT_INT_MAX
#define FT_INT_MAX INT_MAX
#endif

#ifndef FT_Err_Ok
#define FT_Err_Ok 0
#endif

// Map FT_THROW(Error) to a small negative error code space
#define Invalid_Argument 1
#define Array_Too_Large 2
#define Out_Of_Memory 3
#define FT_THROW(e) (-(e))

#define FT_ASSERT(cond) ((void)0)
#define FT_MEM_ZERO(dst, size) memset((dst), 0, (size_t)(size))

typedef struct FT_MemoryRec_ FT_MemoryRec, *FT_Memory;
struct FT_MemoryRec_ {
    void* (*alloc)(FT_Memory, FT_Long size);
    void* (*realloc)(FT_Memory, FT_Long cur_size, FT_Long new_size, void* block);
    void  (*free)(FT_Memory, void* block);
};

