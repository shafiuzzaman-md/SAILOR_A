#include <stdint.h>
#include <stddef.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <string.h>
// klee removed for replay

#ifndef FZ_COLORSPACE_INDEXED
#define FZ_COLORSPACE_INDEXED 1
#endif
#ifndef FZ_COLORSPACE_SEPARATION
#define FZ_COLORSPACE_SEPARATION 2
#endif
#ifndef FZ_COLORSPACE_BGR
#define FZ_COLORSPACE_BGR 3
#endif

/* Mirror minimal types from harness/colorspace.c */
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_color_params { int dummy; } fz_color_params;

typedef struct fz_colorspace {
    int type;
    union {
        struct { unsigned char *md5; } icc;
        struct { struct fz_colorspace *base; } indexed;
        struct { struct fz_colorspace *base; } separation;
    } u;
} fz_colorspace;

typedef struct fz_color_converter {
    void *convert;
    void *convert_via;
    fz_colorspace *ss;
    fz_colorspace *ss_via;
    fz_colorspace *ds;
    void *link;
} fz_color_converter;

/* Entry function prototype (implemented in harness) */
void fz_find_color_converter(fz_context *ctx, fz_color_converter *cc, fz_colorspace *ss, fz_colorspace *ds, fz_colorspace *is, fz_color_params params);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate context and converter
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_color_converter *cc = (fz_color_converter *)calloc(1, sizeof(fz_color_converter));

    // Allocate source and destination colorspaces
    fz_colorspace *ss = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));
    fz_colorspace *ds = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));

    // Prepare a too-small md5 buffer (8 bytes instead of required 16) to induce overflow on memcpy
    unsigned char *md5_src = (unsigned char *)malloc(8);
    { memcpy(md5_src, fuzz_data + 0, 8); };

    // Set up the source colorspace ICC md5 pointer
    ss->u.icc.md5 = md5_src;

    // Construct params
    fz_color_params params; memset(&params, 0, sizeof(params));

    // Call entry (neutralized) which directly calls the vulnerable function
    fz_find_color_converter(ctx, cc, ss, ds, NULL, params);

    return 0;
}
