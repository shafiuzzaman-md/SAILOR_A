/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for pex_run -> pex_run_in_environment */
#include <stdlib.h>
#include <stdint.h>

#ifndef READ_PORT
#define READ_PORT 0
#endif
#ifndef WRITE_PORT
#define WRITE_PORT 1
#endif

/* Minimal forward decls */
struct pex_funcs { int dummy; };
struct pex_obj {
    int next_input;                 /* used by vulnerable path */
    const struct pex_funcs *funcs;  /* present in real struct; unused here */
};

/* Entry: MUST directly call vulnerable function with no guards */
const char * pex_run(struct pex_obj *obj, int flags, const char *executable,
                     char * const * argv, const char *orig_outname, const char *errname,
                     int *err);

const char * pex_run_in_environment(struct pex_obj *obj, int flags, const char *executable,
                                    char * const * argv, char * const * env,
                                    const char *orig_outname, const char *errname,
                                    int *err);

const char * pex_run(struct pex_obj *obj, int flags, const char *executable,
                     char * const * argv, const char *orig_outname, const char *errname,
