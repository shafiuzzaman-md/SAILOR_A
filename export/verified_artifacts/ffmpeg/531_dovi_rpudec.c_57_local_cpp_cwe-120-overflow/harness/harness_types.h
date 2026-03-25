/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Minimal local stand-ins for referenced FFmpeg types */
typedef struct AVDOVIDecoderConfigurationRecord { unsigned char _pad[64]; } AVDOVIDecoderConfigurationRecord;
typedef struct AVDOVIRpuDataHeader { unsigned char _pad[64]; } AVDOVIRpuDataHeader;
typedef struct AVDOVIDataMapping { unsigned char _pad[64]; } AVDOVIDataMapping;
typedef struct AVDOVIColorMetadata { unsigned char _pad[64]; } AVDOVIColorMetadata;
typedef struct DOVIExt { unsigned char _pad[64]; } DOVIExt;

#ifndef FF_DOVI_AUTOMATIC
#define FF_DOVI_AUTOMATIC -1
#endif

typedef struct DOVIContext {
    void *logctx;
    int enable;
    AVDOVIDecoderConfigurationRecord cfg;
    AVDOVIRpuDataHeader header;
    const AVDOVIDataMapping *mapping;
    const AVDOVIColorMetadata *color;
    DOVIExt *ext_blocks;
} DOVIContext;

/* VULNERABLE FUNCTION (neutralized to only keep suspected sink pattern) */
