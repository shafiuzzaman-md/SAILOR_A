/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Local macro/const definitions (avoid project headers)
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif
#ifndef ERR
#define ERR(handle, fmt, ...) printf(fmt, __VA_ARGS__)
#endif

// Local type stand-ins (match names used in target code)
typedef struct sepol_handle { int dummy; } sepol_handle_t;

struct sepol_bool { char *name; int value; };
typedef struct sepol_bool sepol_bool_t;

struct sepol_bool_key { char *name; };
typedef struct sepol_bool_key sepol_bool_key_t;

