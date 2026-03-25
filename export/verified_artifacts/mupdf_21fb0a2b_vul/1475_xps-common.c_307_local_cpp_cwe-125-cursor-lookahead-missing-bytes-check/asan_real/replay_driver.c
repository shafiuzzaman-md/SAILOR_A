// NO_HARNESS_TYPES
#include <stddef.h>
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Match the minimal shims defined in harness/xps-common.c
typedef struct { float a, b, c, d, e, f; } fz_matrix;
typedef struct { float x0, y0, x1, y1; } fz_rect;
typedef struct { int dummy; } fz_context;
typedef struct { int dummy; } xps_document;
typedef struct { int dummy; } xps_resource;
typedef struct { int dummy; } fz_xml;

// Entry prototype (must match harness exactly)
void xps_begin_opacity(fz_context *ctx, xps_document *doc, fz_matrix ctm, fz_rect area,
                       char *base_uri, xps_resource *dict, char *opacity_att, fz_xml *opacity_mask_tag);

// Global buffer pointer consumed by harness entry
float *g_samples;

int main() {
    // Concrete allocations
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    xps_document *doc = (xps_document *)calloc(1, sizeof(xps_document));
    xps_resource *dict = (xps_resource *)calloc(1, sizeof(xps_resource));
    fz_xml *opacity_mask_tag = (fz_xml *)calloc(1, sizeof(fz_xml));

    // Allocate a TOO-SMALL samples buffer to force OOB at samples[2]
    // 2 floats -> indices 0 and 1 valid; samples[2] is OOB
    float *small = (float *)malloc(2 * sizeof(float));
    { static const unsigned char samples_small_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(small, samples_small_data, (2 * sizeof(float) < sizeof(samples_small_data)) ? 2 * sizeof(float) : sizeof(samples_small_data)); };
    g_samples = small;

    // Symbolic inputs
    char base_uri[32];
    { static const unsigned char base_uri_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(base_uri, base_uri_data, (sizeof(base_uri) < sizeof(base_uri_data)) ? sizeof(base_uri) : sizeof(base_uri_data)); };
    base_uri[sizeof(base_uri)-1] = '\0';

    // Ensure opacity_att is non-NULL so harness passes it through
    char opacity_att[32];
    { static const unsigned char opacity_att_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(opacity_att, opacity_att_data, (sizeof(opacity_att) < sizeof(opacity_att_data)) ? sizeof(opacity_att) : sizeof(opacity_att_data)); };
    opacity_att[sizeof(opacity_att)-1] = '\0';

    fz_matrix ctm;
    { static const unsigned char ctm_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&ctm, ctm_data, (sizeof(ctm) < sizeof(ctm_data)) ? sizeof(ctm) : sizeof(ctm_data)); };
    fz_rect area;
    { static const unsigned char area_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&area, area_data, (sizeof(area) < sizeof(area_data)) ? sizeof(area) : sizeof(area_data)); };

    // Direct call to entry (neutralized to call xps_parse_color)
    xps_begin_opacity(ctx, doc, ctm, area, base_uri, dict, opacity_att, opacity_mask_tag);
    return 0;
}
