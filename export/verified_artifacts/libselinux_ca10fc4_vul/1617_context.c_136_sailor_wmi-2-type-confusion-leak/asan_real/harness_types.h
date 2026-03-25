/* AUTO-GENERATED from harness preamble */
#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define COMP_USER  0
#define COMP_ROLE  1
#define COMP_TYPE  2
#define COMP_RANGE 3

typedef struct {
    char *current_str;    /* This is made up-to-date only when needed */
    char *(component[4]);
} context_private_t;

typedef struct context_s {
    context_private_t *ptr;
} *context_t;
