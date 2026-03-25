/* AUTO-GENERATED from harness preamble */
#pragma once

#include "jpeglib.h"

// Minimal local typedefs/macros to satisfy references in the neutralized slice
#ifndef INT32
typedef int INT32;
#endif
#ifndef ISLOW_MULT_TYPE
// ISLOW_MULT_TYPE is typically an INT16 in IJG; exact width not critical here
typedef short ISLOW_MULT_TYPE;
#endif

// Neutralized vulnerable function: keep signature and the vulnerable statement verbatim
void jpeg_idct_15x15 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                     JCOEFPTR coef_block,
