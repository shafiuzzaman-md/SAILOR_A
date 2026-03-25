/* Minimal sliced harness for av_adts_header_parse targeting memcpy overflow at adts_parser.c:38 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

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
int av_adts_header_parse(const uint8_t *buf, uint32_t *samples, uint8_t *frames)
{
    uint8_t tmpbuf[AV_AAC_ADTS_HEADER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    AACADTSHeaderInfo hdr;
    int err;
    /* KEEP the exact vulnerable memcpy statement from source */
    memcpy(tmpbuf, buf, AV_AAC_ADTS_HEADER_SIZE);
    /* Universal sink assertion placed AFTER the vulnerable statement */
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}

/* Mandatory simple pass-through entry function with NO guards */
int entry_func(const uint8_t *buf, uint32_t *samples, uint8_t *frames) {
    av_adts_header_parse(buf, samples, frames);
    return 0;
}
