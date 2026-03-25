// Combined reproducer for 8996_ivi.c_406_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// === driver.c ===
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
