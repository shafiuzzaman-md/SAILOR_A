/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef FZ_LOCK_ALLOC
#define FZ_LOCK_ALLOC 0
#endif

/* Minimal type definitions to match signatures and fields used on path */
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_page {
    struct fz_page *next;
    struct fz_page **prev; /* matches usage: *page->prev = page->next; */
    int number;
} fz_page;

typedef struct {
    struct { fz_page *open; } super; /* doc->super.open */
} pdf_document;

/* No-op locks to satisfy calls */

/* Vulnerable function (neutralized to keep only the target path) */
