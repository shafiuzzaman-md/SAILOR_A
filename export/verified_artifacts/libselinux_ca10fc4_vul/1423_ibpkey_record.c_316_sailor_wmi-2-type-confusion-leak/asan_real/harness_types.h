/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal local definitions to avoid including project headers
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

// Dummy forward decls
typedef struct sepol_handle sepol_handle_t;
typedef struct sepol_context sepol_context_t;

typedef struct sepol_ibpkey {
    /* Subnet prefix */
    uint64_t subnet_prefix;
    /* Low - High range. Same for single ibpkeys. */
    int low, high;
    /* Context */
    sepol_context_t *con;
} sepol_ibpkey_t;

// External functions (stubbed elsewhere or auto-stubbed)

// Logging macro neutralized
