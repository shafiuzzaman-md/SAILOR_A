/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef FT_ERROR_DEFINED
#define FT_ERROR_DEFINED
typedef int FT_Error;
#endif
#ifndef FT_INT_DEFINED
#define FT_INT_DEFINED
typedef int FT_Int;
#endif
#ifndef FT_UINT_DEFINED
#define FT_UINT_DEFINED
typedef unsigned int FT_UInt;
#endif

/* Minimal PS_Driver and FT_Module model sufficient for this slice */
typedef struct PS_DriverRec_ {
    FT_Int darken_params[8];
    FT_UInt hinting_engine;
} PS_DriverRec_, *PS_Driver;

typedef void* FT_Module; /* casted to PS_Driver in the function */

/* Vulnerable function (neutralized to keep only the target case body) */
static FT_Error ps_property_get( FT_Module    module,         /* PS_Driver */
                                 const char*  property_name,
