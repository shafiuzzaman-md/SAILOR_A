#include <stddef.h>
// Combined reproducer for 3103_frame.c_370_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: probe (auto-detected external) */
int probe() { return 0; }

/* PROACTIVE: source (auto-detected external) */
int source() { return 0; }

/* PROACTIVE: the (auto-detected external) */
int the() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal matching types (must match harness/frame.c layout)
typedef struct AVFrameSideData {
    uint8_t *data;
    size_t   size;
    void    *buf;
    int      type;
    struct AVFrameSideData *ref;
} AVFrameSideData;

typedef struct AVFrame {
    AVFrameSideData **side_data;
    int nb_side_data;
} AVFrame;

// Entry prototype from harness
int av_frame_copy_props(AVFrame *dst, const AVFrame *src);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate frames
    AVFrame *src = (AVFrame *)calloc(1, sizeof(AVFrame));
    AVFrame *dst = (AVFrame *)calloc(1, sizeof(AVFrame));

    // Initialize destination side data list as empty
    dst->side_data = NULL;
    dst->nb_side_data = 0;

    // Set up one side_data entry in src to drive the path to the sink
    src->nb_side_data = 1;
    src->side_data = (AVFrameSideData **)calloc(1, sizeof(*src->side_data));
    AVFrameSideData *sd0 = (AVFrameSideData *)calloc(1, sizeof(AVFrameSideData));
    src->side_data[0] = sd0;

    // Non-refcounted path: ensure buf == NULL
    sd0->buf = NULL;
    sd0->type = 1; // arbitrary type

    // Choose a size larger than the source buffer to trigger OOB in memcpy
    size_t sz = 256;
    sd0->size = sz;

    // Back source data with a smaller real buffer; make contents symbolic
    uint8_t *srcbuf = (uint8_t *)malloc(32); // smaller than sz
    memcpy(srcbuf, fuzz_data + (0), 32);
    sd0->data = srcbuf; // memcpy will read sz bytes from here (OOB)

    // Call entry (neutralized pass-through to vulnerable function)
    av_frame_copy_props(dst, src);

    return 0;
}
