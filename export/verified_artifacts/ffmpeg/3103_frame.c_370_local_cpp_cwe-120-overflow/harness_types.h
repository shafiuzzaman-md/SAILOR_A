/* AUTO-GENERATED from harness preamble */
#pragma once

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
