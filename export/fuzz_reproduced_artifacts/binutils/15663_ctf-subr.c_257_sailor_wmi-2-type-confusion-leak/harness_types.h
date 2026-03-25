/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _
#define _(x) (x)
#endif

/* Minimal local type defs to satisfy compilation */
typedef struct ctf_list { struct ctf_list *next; } ctf_list_t;
typedef struct ctf_dict { ctf_list_t ctf_errs_warnings; } ctf_dict_t;
typedef struct ctf_err_warning { int cew_is_warning; char *cew_text; } ctf_err_warning_t;

/* Externals to be stubbed */

