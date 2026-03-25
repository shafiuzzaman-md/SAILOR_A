// Combined reproducer for 486_codec_par.c_191_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: AVERROR (auto-detected external) */
int AVERROR() { return 0; }

/* PROACTIVE: Stubs (auto-detected external) */
int Stubs() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
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
