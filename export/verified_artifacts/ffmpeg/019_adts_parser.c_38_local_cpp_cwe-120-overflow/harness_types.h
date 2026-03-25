/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for av_adts_header_parse targeting memcpy overflow at adts_parser.c:38 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef AV_AAC_ADTS_HEADER_SIZE
#define AV_AAC_ADTS_HEADER_SIZE 7
#endif
#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#endif
#ifndef AVERROR
#define AVERROR(e) (-1)
#endif
#ifndef EINVAL
#define EINVAL 22
#endif

typedef struct AACADTSHeaderInfo {
    uint32_t sample_rate;
    uint32_t samples;
    uint32_t bit_rate;
    uint8_t  crc_absent;
    uint8_t  object_type;
    uint8_t  sampling_index;
    uint8_t  chan_config;
    uint8_t  num_aac_frames;
    uint32_t frame_length;
} AACADTSHeaderInfo;

/* Neutralized vulnerable function: keep signature and the exact vulnerable statement */
