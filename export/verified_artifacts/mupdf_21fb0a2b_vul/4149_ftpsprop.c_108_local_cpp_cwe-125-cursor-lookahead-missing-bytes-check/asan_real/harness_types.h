/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ftpsprop.c ps_property_set CWE-125 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Minimal local FreeType-like typedefs/macros to keep this standalone */
typedef int FT_Error;
typedef int FT_Bool;
typedef int FT_Int;
typedef void* FT_Module; /* we will pass a PS_DriverRec_* casted to FT_Module */

#ifndef FT_Err_Ok
#define FT_Err_Ok 0
#endif
#ifndef FT_THROW
#define FT_THROW(x) (-1)
#endif
#ifndef ft_strcmp
#define ft_strcmp strcmp
#endif
#ifndef ft_strtol
#define ft_strtol strtol
#endif
#ifndef FT_UNUSED
#define FT_UNUSED(x) (void)(x)
#endif

/* Minimal PS_Driver type with the field we touch */
typedef struct PS_DriverRec_ {
    FT_Int darken_params[8];
    FT_Int random_seed;
} *PS_Driver;

/* Vulnerable function slice from ftpsprop.c */
FT_Error ps_property_set( FT_Module    module,         /* PS_Driver */
                          const char*  property_name,
                          const void*  value,
                          FT_Bool      value_is_string )
{
    FT_Error   error  = FT_Err_Ok;
    PS_Driver  driver = (PS_Driver)module;

#ifndef FT_CONFIG_OPTION_ENVIRONMENT_PROPERTIES
#endif

    if ( !ft_strcmp( property_name, "darkening-parameters" ) )
    {
        FT_Int*  darken_params;
        FT_Int   x1, y1, x2, y2, x3, y3, x4, y4;

#ifdef FT_CONFIG_OPTION_ENVIRONMENT_PROPERTIES
