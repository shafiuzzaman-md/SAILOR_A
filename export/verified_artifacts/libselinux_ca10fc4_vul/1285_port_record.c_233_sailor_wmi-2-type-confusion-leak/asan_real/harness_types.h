/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdarg.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif

/* Minimal type definitions */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_port {
    int low;
    int high;
    int proto;
    void *con;
} sepol_port_t;

/* Decls for helpers/stubs provided elsewhere */

