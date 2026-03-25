#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#endif

// Minimal types needed for the path to the sink
typedef struct AVCodecParameters {
    uint8_t *extradata;
    size_t   extradata_size;
} AVCodecParameters;

typedef struct AVCodecContext {
    uint8_t *extradata;
    size_t   extradata_size;
} AVCodecContext;

// Stubs (simple over-approximations)
static void av_freep(void *ptrarg) {
    void **pp = (void **)ptrarg;
    if (pp && *pp) { free(*pp); *pp = NULL; }
}

static void *av_mallocz(size_t size) {
    void *p = malloc(size);
    if (p) memset(p, 0, size);
    return p;
}

// Vulnerable function (neutralized to only keep the extradata path with the sink)
int avcodec_parameters_to_context(AVCodecContext *codec, const AVCodecParameters *par) {
    av_freep(&codec->extradata);
    codec->extradata_size = 0;
    if (par->extradata) {
        codec->extradata = av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return -12; // AVERROR(ENOMEM) placeholder
        // Vulnerable statement from codec_par.c:191
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        klee_assert(0 && "SAILOR_SINK_REACHED");
        codec->extradata_size = par->extradata_size;
    }
    return 0;
}

// Mandatory pass-through entry function
int entry_func(AVCodecContext *codec, const AVCodecParameters *par) {
    avcodec_parameters_to_context(codec, par);
    return 0;
}
