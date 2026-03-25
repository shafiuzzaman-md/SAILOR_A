/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef BFD_SUPPORTS_PLUGINS
#define BFD_SUPPORTS_PLUGINS 0
#endif

/* Minimal type shims to satisfy signatures */
typedef struct bfd_target { int dummy; } bfd_target;
typedef int bfd_format;
typedef struct bfd { int target_defaulted; } bfd;

/* Dummy global referenced in condition */
const bfd_target binary_vec = { 0 };

