#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

// harness_entry is defined in the harness and directly calls ebitmap_cpy
int ebitmap_cpy(ebitmap_t *dst, const ebitmap_t *src);

int main() {
    // Allocate destination bitmap concretely
    ebitmap_t *dst = (ebitmap_t *)malloc(sizeof(ebitmap_t));
    if (!dst) return 0;

    // Allocate source bitmap, then free it to create a UAF scenario
    ebitmap_t *src = (ebitmap_t *)malloc(sizeof(ebitmap_t));
    if (!src) return 0;

    // Optionally initialize some bytes (not required for UAF itself)
    { static const unsigned char src_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, src_bytes_data, (sizeof(*src) < sizeof(src_bytes_data)) ? sizeof(*src) : sizeof(src_bytes_data)); };

    // Free src so that ebitmap_cpy will dereference a freed pointer (UAF)
    free(src);

    // Call entry: ebitmap_cpy will read src->node (use-after-free)
    ebitmap_cpy(dst, (const ebitmap_t *)src);

    return 0;
}
