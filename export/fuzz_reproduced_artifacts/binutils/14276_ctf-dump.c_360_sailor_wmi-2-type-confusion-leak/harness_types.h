/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local type defs (sliced)
typedef struct ctf_dict { int dummy; } ctf_dict_t;
typedef struct ctf_dump_state { int dummy; } ctf_dump_state_t;
typedef int ctf_sect_names_t;
typedef void ctf_dump_decorate_f(void);

typedef struct ctf_header {
    uint32_t cth_parlabel;
    uint32_t cth_parname;
    uint32_t cth_cuname;
    uint32_t cth_lbloff;
    uint32_t cth_objtoff;
    uint32_t cth_funcoff;
    uint32_t cth_objtidxoff;
} ctf_header;

// Stub: only to keep the call site intact; the UAF happens when evaluating hp->cth_parname
