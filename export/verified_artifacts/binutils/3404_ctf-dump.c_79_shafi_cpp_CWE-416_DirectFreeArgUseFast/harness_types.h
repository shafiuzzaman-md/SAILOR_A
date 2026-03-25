/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for libctf ctf-dump UAF/DF reachability */
#include <stdlib.h>
#include <stdint.h>

/* Minimal stand-ins for project types used by the signatures */
typedef struct ctf_dict ctf_dict_t;  /* opaque */
typedef int ctf_sect_names_t;        /* enum in real code */
typedef void (ctf_dump_decorate_f)(void); /* unused here */

typedef struct ctf_list { struct ctf_list *l_next; } ctf_list_t;

typedef struct ctf_dump_item {
    ctf_list_t cdi_list;
    char *cdi_item;
} ctf_dump_item_t;

typedef struct ctf_dump_state {
    ctf_sect_names_t cds_sect;
    ctf_dict_t *cds_fp;
    ctf_dump_item_t *cds_current;
    ctf_list_t cds_items;
} ctf_dump_state_t;

/* Extern: provided by stubs.c */

/* Vulnerable function: keep only the path to the sink. */
