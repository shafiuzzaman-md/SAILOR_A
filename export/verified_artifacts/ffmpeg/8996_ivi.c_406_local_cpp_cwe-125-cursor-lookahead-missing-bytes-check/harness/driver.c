// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef av_cold
#define av_cold
#endif

typedef struct IVIBandDesc IVIBandDesc; // opaque here

typedef struct IVIPlaneDesc {
    uint16_t    width;
    uint16_t    height;
    uint8_t     num_bands;  // number of bands this plane subdivided into
    IVIBandDesc *bands;     // array of band descriptors
} IVIPlaneDesc;

// Prototype to match the harness function
av_cold int ff_ivi_init_tiles(IVIPlaneDesc *planes, int tile_width, int tile_height);

int main() {
    // Allocate undersized buffer so reading planes[0].num_bands (offset 4) goes OOB
    unsigned char *buf = (unsigned char *)malloc(1);
    if (!buf) return 0;
    klee_make_symbolic(buf, 1, "planes_byte0");

    IVIPlaneDesc *planes = (IVIPlaneDesc *)buf;

    int tile_width = 2;
    int tile_height = 2;

    ff_ivi_init_tiles(planes, tile_width, tile_height);
    return 0;
}
