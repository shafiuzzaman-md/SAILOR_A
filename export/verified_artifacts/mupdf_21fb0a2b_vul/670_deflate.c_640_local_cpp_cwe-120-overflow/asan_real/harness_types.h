/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef ZEXPORT
#define ZEXPORT
#endif

/* Minimal local types to avoid pulling full zlib headers */
typedef struct deflate_state {
    int level;
    unsigned int matches;
    void *head;           /* hash head buffer */
    size_t hash_size;     /* number of entries in head (units of uint16_t) */
    /* params set by configuration_table */
    int max_lazy_match;
    int good_match;
    int nice_match;
    int max_chain_length;
    int strategy;
} deflate_state;

typedef struct z_stream_local {
    deflate_state *state;
    unsigned int avail_in;
} z_stream_local;

typedef z_stream_local* z_streamp;

/* Minimal config table to satisfy assignments */
typedef struct {
    int good_length;
    int max_lazy;
    int nice_length;
    int max_chain;
} config_t;

#ifndef MAX_LEVELS
#define MAX_LEVELS 10
#endif

static const config_t configuration_table[MAX_LEVELS] = {
    {0,0,0,0}, {1,1,1,1}, {2,2,2,2}, {3,3,3,3}, {4,4,4,4},
    {5,5,5,5}, {6,6,6,6}, {7,7,7,7}, {8,8,8,8}, {9,9,9,9}
};

/* Stubs matching names used around the site */
#define slide_hash(s) ((void)0)

