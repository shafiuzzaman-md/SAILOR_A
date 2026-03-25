/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for pdf_authenticate_password -> pdf_authenticate_owner_password */
#include <stddef.h>
#include <string.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Minimal type defs needed for the path */
typedef struct fz_context { int dummy; } fz_context;

typedef struct pdf_crypt {
    int r;              /* revision */
    int length;         /* key length in bits (not used here) */
    unsigned char *o;   /* owner key pointer */
} pdf_crypt;

typedef struct pdf_document {
    pdf_crypt *crypt;
} pdf_document;

/* Vulnerable function (neutralized to keep only the target case and sink) */
