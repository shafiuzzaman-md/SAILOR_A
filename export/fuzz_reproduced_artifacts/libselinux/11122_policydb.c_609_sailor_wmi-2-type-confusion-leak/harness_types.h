/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for policydb.c:609 — type_datum_init */
#include <stddef.h>
#include <string.h>

/* Minimal local types to support the vulnerable statement */
typedef struct {
    /* real ebitmap_t has internal fields; we only need a concrete size */
    unsigned long dummy[2];
} ebitmap_t;

typedef struct {
    ebitmap_t types;  /* field accessed at the vulnerable line */
} type_datum_t;

