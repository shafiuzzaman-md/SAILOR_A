/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/* Minimal local typedefs to avoid pulling project headers */
typedef struct sepol_handle sepol_handle_t;
typedef struct policydb policydb_t;

struct sepol_handle {
    int msg_level;
    const char *msg_channel;
    const char *msg_fname;
#ifdef __GNUC__
    __attribute__ ((format(printf, 3, 4)))
#endif
    void (*msg_callback)(void *varg, sepol_handle_t *handle, const char *fmt, ...);
    void *msg_callback_arg;
    int disable_dontaudit;
    int expand_consume_base;
    int preserve_tunables;
};

struct policydb { int dummy; };

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif

/* Local ERR macro: mirrors real behavior by reading fields of 'handle'.
 * Keeping the macro call text EXACT in the function to preserve mapping.
 */
#define ERR(h, fmt, ...) do { \
    /* Read from the (possibly freed) handle fields to materialize the bug */ \
    if ((h) && (h)->msg_callback) { \
        /* We do not care about actually invoking the callback; the read above is enough */ \
        /* Optionally touch the arg, too, to ensure multiple field reads */ \
        (void)((h)->msg_callback_arg); \
    } \
} while (0)

