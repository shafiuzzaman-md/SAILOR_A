/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

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
int ff_dovi_rpu_parse(DOVIContext *s, const uint8_t *rpu, size_t rpu_size, int err_recognition)
{
    (void)s; (void)err_recognition;
    /* Target pattern: unchecked memcpy length (models dovi_rpudec.c:57 class) */
    unsigned char dst[32];
    /* Vulnerable statement (unchecked length) */
    memcpy(dst, rpu, rpu_size);
    /* Universal sink assertion: fires if memcpy didn't crash */
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}

/* ENTRY function: MUST be a direct pass-through to vul func with no guards */
int dovi_entry(DOVIContext *s, const uint8_t *rpu, size_t rpu_size, int err_recognition)
{
    ff_dovi_rpu_parse(s, rpu, rpu_size, err_recognition);
    return 0;
}
