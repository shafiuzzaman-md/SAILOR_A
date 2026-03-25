#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

int entry_func(AVCodecContext *codec, const AVCodecParameters *par);

int main() {
    // Allocate concrete structs
    AVCodecContext *codec = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));
    AVCodecParameters *par = (AVCodecParameters *)calloc(1, sizeof(AVCodecParameters));

    // Prepare source extradata buffer with concrete size, symbolic content
    const size_t SRC_SIZE = 64;  // concrete to keep malloc sizes concrete
    uint8_t *src = (uint8_t *)malloc(SRC_SIZE);
    klee_make_symbolic(src, SRC_SIZE, "par_extradata_bytes");

    par->extradata = src;
    par->extradata_size = SRC_SIZE + 256;  // larger than source to trigger OOB read at memcpy

    // Call entry (pass-through to vulnerable function)
    entry_func(codec, par);

    return 0;
}
