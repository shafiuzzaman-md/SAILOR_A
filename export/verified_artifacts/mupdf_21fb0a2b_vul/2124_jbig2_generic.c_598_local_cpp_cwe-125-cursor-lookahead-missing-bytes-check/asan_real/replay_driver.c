#include <string.h>
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

/* Minimal replicas matching harness types */
typedef struct Jbig2Ctx { int dummy; } Jbig2Ctx;
typedef struct Jbig2Segment { int number; } Jbig2Segment;

typedef struct Jbig2Image {
    int width;
    int height;
    int stride;
    uint8_t *data;
} Jbig2Image;

typedef struct Jbig2ArithState { int dummy; } Jbig2ArithState;
typedef uint16_t Jbig2ArithCx;

typedef struct Jbig2GenericRegionParams {
    int MMR;
    int TPGDON;
    int GBTEMPLATE;
    int USESKIP;
    int GBW;
    int GBH;
    int8_t gbat[8];
    Jbig2Image *SKIP;
} Jbig2GenericRegionParams;

/* Entry prototype (from harness) */
int jbig2_decode_generic_region(Jbig2Ctx *ctx,
                                Jbig2Segment *segment,
                                const Jbig2GenericRegionParams *params,
                                Jbig2ArithState *as,
                                Jbig2Image *image,
                                Jbig2ArithCx *GB_stats);

int main() {
    // Allocate core objects
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *seg = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));
    Jbig2ArithState *as = (Jbig2ArithState *)calloc(1, sizeof(Jbig2ArithState));
    Jbig2Image *img = (Jbig2Image *)calloc(1, sizeof(Jbig2Image));
    Jbig2GenericRegionParams *params = (Jbig2GenericRegionParams *)calloc(1, sizeof(Jbig2GenericRegionParams));

    // Initialize segment
    seg->number = 1;

    // Set region parameters to reach template1 path and trigger second-byte read
    params->MMR = 0;
    params->TPGDON = 0;
    params->GBTEMPLATE = 1; // target template path
    params->USESKIP = 0;
    params->GBW = 16;  // >8 to execute 'ppd |= *ppline++'
    params->GBH = 3;   // >=3 so y>1 branch executes
    params->gbat[0] = +3; params->gbat[1] = -1;
    params->gbat[2] = -3; params->gbat[3] = -1;
    params->gbat[4] = +2; params->gbat[5] = -2;
    params->gbat[6] = -2; params->gbat[7] = -2;
    params->SKIP = NULL;

    // Prepare image matching params
    int stride = (params->GBW + 7) >> 3; // bytes per line (will be 2)
    img->width = params->GBW;
    img->height = params->GBH;
    img->stride = stride;

    // Deliberately UNDER-ALLOCATE the image buffer to trigger OOB on second byte
    size_t datasz = 1; // only 1 byte available; second deref at *(ppline+1) will OOB
    img->data = (uint8_t *)calloc(1, datasz);
    { static const unsigned char img_data_data[] = {0x00}; memcpy(img->data, img_data_data, (datasz < sizeof(img_data_data)) ? datasz : sizeof(img_data_data)); };

    // GB_stats buffer (concrete)
    Jbig2ArithCx *GB_stats = (Jbig2ArithCx *)calloc(2048, sizeof(Jbig2ArithCx));

    // Call entry (neutralized pass-through to vulnerable function)
    (void)jbig2_decode_generic_region(ctx, seg, params, as, img, GB_stats);

    return 0;
}
