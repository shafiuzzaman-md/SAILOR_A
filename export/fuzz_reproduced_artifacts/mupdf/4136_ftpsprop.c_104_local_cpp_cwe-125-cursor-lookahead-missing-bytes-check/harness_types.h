/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Minimal local typedefs/macros to stand in for FreeType types
typedef int FT_Int;
typedef int FT_Error;
typedef int FT_Bool;
typedef void* FT_Module;

#ifndef FT_THROW
#define FT_THROW(x) (-1)
#endif

// Minimal driver struct with only the needed field
typedef struct PS_DriverRec_ {
    FT_Int darken_params[8];
    FT_Int no_stem_darkening;
} PS_DriverRec, *PS_Driver;

// Entry function: MUST be a direct pass-through to the vulnerable function

// Vulnerable function (neutralized) reconstructed from ftpsprop.c excerpt
