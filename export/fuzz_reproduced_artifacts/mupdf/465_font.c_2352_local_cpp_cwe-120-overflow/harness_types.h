/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal stand-ins for project types
typedef struct fz_context_s { int dummy; } fz_context;
typedef struct fz_font_s { int dummy; } fz_font;
typedef struct fz_buffer_s { unsigned char *data; size_t size; } fz_buffer;

// Entry == Vulnerable function (neutralized to directly perform the vulnerable operation)
fz_buffer *
