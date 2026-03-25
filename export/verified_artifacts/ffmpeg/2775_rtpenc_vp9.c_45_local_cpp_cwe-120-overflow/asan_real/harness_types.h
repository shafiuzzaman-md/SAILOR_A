/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef RTP_VP9_DESC_REQUIRED_SIZE
#define RTP_VP9_DESC_REQUIRED_SIZE 1
#endif
#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

typedef struct AVFormatContext {
    void *priv_data;
} AVFormatContext;

typedef struct RTPMuxContext {
    uint32_t cur_timestamp;
    uint32_t timestamp;
    uint8_t *buf;
    uint8_t *buf_ptr;
    int max_payload_size;
} RTPMuxContext;

// External stubbed elsewhere

// Vulnerable function (neutralized minimal body, but preserving the vulnerable statement verbatim)
