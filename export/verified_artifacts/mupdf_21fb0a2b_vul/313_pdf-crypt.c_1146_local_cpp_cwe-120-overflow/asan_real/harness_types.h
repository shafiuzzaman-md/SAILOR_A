/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal placeholder types to satisfy signatures
typedef struct { int dummy; } fz_context;

typedef struct {
    struct { int method; } strf;
} pdf_crypt;

typedef struct {
    unsigned char *str; // string buffer
    int len;            // string length
} pdf_obj;

// Vulnerable function (neutralized to the essential path). Keep exact sink line.
