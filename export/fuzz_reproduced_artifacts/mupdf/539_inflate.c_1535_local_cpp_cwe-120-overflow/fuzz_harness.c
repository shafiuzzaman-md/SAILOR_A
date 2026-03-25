#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef ENOUGH
#define ENOUGH 2048
#endif

int inflateCopy(z_streamp dest, z_streamp source); // from harness

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1568) return 0;
    // Allocate z_stream structs
    z_stream *src = (z_stream *)calloc(1, sizeof(z_stream));
    z_stream *dst = (z_stream *)calloc(1, sizeof(z_stream));

    // Allocate inflate_state for source and destination
    struct inflate_state *src_state = (struct inflate_state *)calloc(1, sizeof(struct inflate_state));
    struct inflate_state *dst_state = (struct inflate_state *)calloc(1, sizeof(struct inflate_state));

    // Backing buffers for code tables
    int *src_codes = (int *)malloc(sizeof(int) * (ENOUGH + 64));
    int *dst_codes = (int *)malloc(sizeof(int) * (ENOUGH + 64));

    // Initialize buffers with symbolic data
    { memcpy(src_codes, fuzz_data + 0, 512); };
    { memcpy(dst_codes, fuzz_data + 512, 512); };

    // Set up source inflate_state fields
    src_state->codes = src_codes;
    src_state->lencode = src_codes + 10;   // inside range [codes, codes+ENOUGH-1]
    src_state->distcode = src_codes + 20;  // arbitrary in-range
    src_state->next = src_codes + 30;      // arbitrary in-range

    // Optional window setup (not reached due to sink assertion, but safe)
    unsigned int wbits = 12; // window size = 4096
    unsigned char *src_window = (unsigned char *)malloc(1u << wbits);
    { memcpy(src_window, fuzz_data + 1024, 512); };
    src_state->window = src_window;
    src_state->wbits = wbits;

    // Destination inflate_state fields
    dst_state->codes = dst_codes;
    dst_state->lencode = dst_codes; // will be set by inflateCopy if taken
    dst_state->distcode = dst_codes;
    dst_state->next = dst_codes;
    size_t dst_sz = 32;
    unsigned char *dst_window = (unsigned char *)malloc(dst_sz);
    { memcpy(dst_window, fuzz_data + 1536, 32); };
    dst_state->window = dst_window;
    dst_state->wbits = wbits;

    // Wire z_stream to states
    src->state = (internal_state *)src_state;
    dst->state = (internal_state *)dst_state;

    // Call entry (pure pass-through to inflateCopy)
    inflateCopy(dst, src);

    return 0;
}
