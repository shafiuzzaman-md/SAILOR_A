#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Prepare cmsContext (unused by our stubbed _cmsContextGetClientChunk)
    cmsContext id = NULL;

    // Allocate plugin object concretely
    cmsPluginRenderingIntent *plugin = (cmsPluginRenderingIntent *)calloc(1, sizeof(cmsPluginRenderingIntent));

    // Small source buffer to provoke over-read by strncpy of 63 bytes
    enum { SRC_SIZE = 8 };
    char *src = (char *)malloc(SRC_SIZE);
    { memcpy(src, fuzz_data + 0, 8); };
    // Do NOT enforce NUL within SRC_SIZE; let strncpy read past SRC_SIZE

    // Fill plugin fields
    plugin->Description = src;   // non-terminated, small buffer
    // Leave Intent and Link unconstrained/zeroed

    // Call entry (pass-through to vulnerable function)
    _cmsRegisterRenderingIntentPlugin(id, (cmsPluginBase *)plugin);

    return 0;
}
