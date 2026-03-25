#include <stdint.h>
#include <klee/klee.h>

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
                              int tile_width, int tile_height)
{
    int p;
    int t_width, t_height;

    for (p = 0; p < 3; p++) {
        t_width  = !p ? tile_width  : (tile_width  + 3) >> 2;
        t_height = !p ? tile_height : (tile_height + 3) >> 2;

        // Vulnerable statement copied verbatim from ivi.c:406
        if (!p && planes[0].num_bands == 4) {
        }
        // Universal sink assertion placed AFTER the vulnerable statement
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }

    return 0;
}
