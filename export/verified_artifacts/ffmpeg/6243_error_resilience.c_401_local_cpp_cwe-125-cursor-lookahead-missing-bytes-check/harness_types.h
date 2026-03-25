/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

// Use the auto-injected harness_types.h for real project types (ERContext, etc.)
// We rely on fields present in the provided struct definitions:
//   - ptrdiff_t mb_stride;
//   - int mb_height;
//   - uint8_t *er_temp_buffer;
// Other fields are unused in this slice.

// Entry: simple pass-through to the vulnerable function (no guards!)
