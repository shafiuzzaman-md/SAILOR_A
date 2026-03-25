#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Prepare cmsContext (unused by our stubbed _cmsContextGetClientChunk)
    cmsContext id = NULL;

    // Allocate plugin object concretely
    cmsPluginRenderingIntent *plugin = (cmsPluginRenderingIntent *)calloc(1, sizeof(cmsPluginRenderingIntent));

    // Small source buffer to provoke over-read by strncpy of 63 bytes
    enum { SRC_SIZE = 8 };
    char *src = (char *)malloc(SRC_SIZE);
    { static const unsigned char plugin_description_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; memcpy(src, plugin_description_data, (SRC_SIZE < sizeof(plugin_description_data)) ? SRC_SIZE : sizeof(plugin_description_data)); };
    // Do NOT enforce NUL within SRC_SIZE; let strncpy read past SRC_SIZE

    // Fill plugin fields
    plugin->Description = src;   // non-terminated, small buffer
    // Leave Intent and Link unconstrained/zeroed

    // Call entry (pass-through to vulnerable function)
    _cmsRegisterRenderingIntentPlugin(id, (cmsPluginBase *)plugin);

    return 0;
}
