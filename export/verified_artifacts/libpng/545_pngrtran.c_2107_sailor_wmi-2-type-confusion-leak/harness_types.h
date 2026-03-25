/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

/* Minimal local typedefs matching the function signature */
typedef struct { int dummy; } png_struct;
typedef struct { int bit_depth; } png_info;

/* Entry == Vulnerable function: keep only the vulnerable read and a reachability sink */
