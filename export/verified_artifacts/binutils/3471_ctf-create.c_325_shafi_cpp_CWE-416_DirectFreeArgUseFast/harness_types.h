/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf_rollback -> ctf_dvd_delete */
#include <stdint.h>
#include <stdlib.h>

/* Minimal type definitions needed by the path */
typedef struct {
    void *head;
} ctf_list_t;

typedef struct ctf_dvdef {
    char *dvd_name;
} ctf_dvdef_t;

typedef struct ctf_dict {
    void *ctf_dvhash;
    ctf_list_t ctf_dvdefs;
} ctf_dict_t;

typedef struct ctf_snapshot_id {
    unsigned long snapshot_id;
    unsigned long dtd_id;
} ctf_snapshot_id_t;

/* External stubs to be provided in stubs.c */

/* Driver-provided DVD pointer (selected by the test) */
extern ctf_dvdef_t *g_dvd;

