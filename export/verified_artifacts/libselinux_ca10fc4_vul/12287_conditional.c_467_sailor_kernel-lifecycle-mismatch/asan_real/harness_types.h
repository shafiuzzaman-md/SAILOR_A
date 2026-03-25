/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>

// Minimal type needed for the vulnerable access
typedef struct cond_av_list {
    struct cond_av_list *next;
    // other fields omitted
} cond_av_list_t;

// Vulnerable function (from conditional.c around line 467)
