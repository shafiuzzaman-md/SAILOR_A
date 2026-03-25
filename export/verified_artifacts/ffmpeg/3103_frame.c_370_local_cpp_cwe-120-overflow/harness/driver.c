// NO_HARNESS_TYPES
#include <klee/klee.h>
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

int main() {
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
    klee_make_symbolic(srcbuf, 32, "srcbuf");
    sd0->data = srcbuf; // memcpy will read sz bytes from here (OOB)

    // Call entry (neutralized pass-through to vulnerable function)
    av_frame_copy_props(dst, src);

    return 0;
}
