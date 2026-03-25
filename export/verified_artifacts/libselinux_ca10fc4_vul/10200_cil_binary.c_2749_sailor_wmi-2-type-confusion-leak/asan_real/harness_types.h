/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR -1
#endif

// Minimal stand-ins for project types we touch
typedef struct policydb policydb_t;  // opaque
struct cil_db;                       // opaque

typedef struct { int dummy; } ebitmap_t;  // minimal; enough for compilation

typedef struct cil_roleallow {
    char *src_str;
    void *src; /* role or attribute */
    char *tgt_str;
    void *tgt; /* role or attribute */
} cil_roleallow_t;

// External function on the path (we provide a stub in stubs.c)

// VULNERABLE FUNCTION — neutralized to keep only the vulnerable statement
