#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Declarations from harness
int entry_func(AVPacket *dst, const AVPacket *src);

int main() {
    // Concrete buffer sizes (no symbolic malloc sizes)
    enum { SRC_DATA_CAP = 16, COPY_SIZE = 64 };

    // Allocate packets
    AVPacket *src = (AVPacket*)calloc(1, sizeof(AVPacket));
    AVPacket *dst = (AVPacket*)calloc(1, sizeof(AVPacket));

    // Ensure we take the !src->buf path
    src->buf = NULL;

    // Allocate a smaller source buffer than the memcpy length to trigger OOB read
    uint8_t *src_data = (uint8_t*)malloc(SRC_DATA_CAP);
    klee_make_symbolic(src_data, SRC_DATA_CAP, "src_data");
    src->data = src_data;

    // Set size larger than allocated source to cause overflow during memcpy
    src->size = COPY_SIZE; // 64 > 16

    // Call entry to reach vulnerable memcpy
    entry_func(dst, src);

    return 0;
}
