#include <stdint.h>
#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef FZ_ENABLE_ICC
#define FZ_ENABLE_ICC 1
#endif
#ifndef FZ_COLORSPACE_BGR
#define FZ_COLORSPACE_BGR 13
#endif

typedef struct fz_context { int dummy; } fz_context;

typedef int fz_color_params; // minimal stand-in

typedef struct fz_colorspace_s {
    int type;
    union {
        struct { unsigned char *md5; } icc;
    } u;
} fz_colorspace;

typedef struct fz_color_converter { int dummy; } fz_color_converter;

extern void fz_find_color_converter(fz_context *ctx, fz_color_converter *cc, fz_colorspace *ss, fz_colorspace *ds, fz_colorspace *is, fz_color_params params);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Concrete allocations for required arguments
    fz_context ctx_obj; fz_context *ctx = &ctx_obj;
    fz_color_converter cc_obj; fz_color_converter *cc = &cc_obj;
    fz_colorspace ss_obj; fz_colorspace *ss = &ss_obj; // not used by sink
    fz_colorspace ds_obj; fz_colorspace *ds = &ds_obj; // used by sink
    fz_colorspace is_obj; fz_colorspace *is = &is_obj; // not used by sink

    // Prepare dst->u.icc.md5 to be a too-small buffer (8 < 16) to trigger OOB read in memcpy
    unsigned char *dst_md5 = (unsigned char *)malloc(8);
    { memcpy(dst_md5, fuzz_data + 0, 8); };

    // Initialize the colorspace objects conservatively
    memset(&ss_obj, 0, sizeof(ss_obj));
    memset(&ds_obj, 0, sizeof(ds_obj));
    memset(&is_obj, 0, sizeof(is_obj));

    ds->u.icc.md5 = dst_md5;
    ds->type = 0; ss->type = 0; is->type = 0;

    // params can be any int; keep symbolic to overapproximate
    fz_color_params params;
    { static const unsigned char params_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&params, params_data, (sizeof(params) < sizeof(params_data)) ? sizeof(params) : sizeof(params_data)); };

    // Call entry (neutralized to directly call fz_find_icc_link)
    fz_find_color_converter(ctx, cc, ss, ds, is, params);
    return 0;
}
