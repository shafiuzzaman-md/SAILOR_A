/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>

#ifndef av_cold
#define av_cold
#endif

typedef struct IVIBandDesc IVIBandDesc;  // opaque

typedef struct IVIPlaneDesc {
    uint16_t    width;
    uint16_t    height;
    uint8_t     num_bands;  ///< number of bands this plane subdivided into
    IVIBandDesc *bands;     ///< array of band descriptors
} IVIPlaneDesc;

// Neutralized vulnerable function: keep signature and the vulnerable statement verbatim
av_cold int ff_ivi_init_tiles(IVIPlaneDesc *planes,
