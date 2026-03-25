#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#endif

// Minimal structs needed
typedef struct AVFrameSideData {
    uint8_t *data;
    size_t   size;
    void    *buf;   // treat as presence flag; non-NULL means refcounted path
    int      type;
    struct AVFrameSideData *ref;
} AVFrameSideData;

typedef struct AVFrame {
    AVFrameSideData **side_data;
    int nb_side_data;
} AVFrame;

// Stub: allocate side data entry and append to dst list
static AVFrameSideData *av_frame_side_data_new(AVFrameSideData ***psd,
                                               int *pnb,
                                               int type,
                                               size_t size,
                                               int flags) {
    (void)flags;
    AVFrameSideData *entry = (AVFrameSideData *)calloc(1, sizeof(AVFrameSideData));
    if (!entry) return NULL;
    entry->type = type;
    entry->size = size;
    entry->buf  = NULL;
    entry->data = NULL;

    int nb = *pnb;
    AVFrameSideData **newarr = (AVFrameSideData **)realloc(*psd, (nb + 1) * sizeof(*newarr));
    if (!newarr) {
        free(entry);
        return NULL;
    }
    newarr[nb] = entry;
    *psd = newarr;
    *pnb = nb + 1;
    return entry;
}

// Allocation stub: fail for large sizes to drive the (!sd_dst->data) branch
static void *av_malloc(size_t size) {
    if (size > 1024) return NULL; // allocation failure for large copies
    return malloc(size);
}

// Entry function must be pass-through (neutralized)
int av_frame_copy_props(AVFrame *dst, const AVFrame *src);
static int frame_copy_props(AVFrame *dst, const AVFrame *src, int force_copy);

int av_frame_copy_props(AVFrame *dst, const AVFrame *src) {
    return frame_copy_props(dst, src, 1);
}

// Vulnerable function (neutralized to the minimal path)
static int frame_copy_props(AVFrame *dst, const AVFrame *src, int force_copy) {
    (void)force_copy;
    int ret = 0;

    // Only handle first side_data entry to reach the sink
    if (src->nb_side_data > 0 && src->side_data && src->side_data[0]) {
        AVFrameSideData *sd_src = src->side_data[0];
        AVFrameSideData *sd_dst = av_frame_side_data_new(&dst->side_data, &dst->nb_side_data,
                                                         sd_src->type, sd_src->size, 0);
        if (!sd_dst)
            return -12; // ENOMEM

        if (sd_src->buf) {
            // Skip refcounted path entirely to stay on target path
        } else {
            sd_dst->data = (uint8_t *)av_malloc(sd_src->size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!sd_dst->data)
                return -12; // ENOMEM
            // Exact vulnerable statement from source (keep verbatim!)
            memcpy(sd_dst->data, sd_src->data, sd_src->size);
            // AFTER — reachability probe (only fires if statement didn't crash)
            klee_assert(0 && "SAILOR_SINK_REACHED");
            memset(sd_dst->data + sd_src->size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        }
    }

    return ret;
}
