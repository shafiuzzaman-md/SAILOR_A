/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

// Minimal type definitions to reach the vulnerable statement
struct TIFFDirectory {
    uint16_t td_fillorder;
    uint16_t td_bitspersample;
    uint16_t td_threshholding;
    uint16_t td_orientation;
    uint16_t td_samplesperpixel;
    uint32_t td_rowsperstrip;
    uint32_t td_tilewidth;
    uint32_t td_tilelength;
    uint32_t td_tiledepth;
    uint16_t td_resolutionunit;
    uint16_t td_sampleformat;
    uint32_t td_imagedepth;
    uint16_t td_ycbcrsubsampling[2];
    uint16_t td_ycbcrpositioning;
};

struct TIFFTagMethods {
    void *vsetfield;
    void *vgetfield;
    void *printdir;
};

struct TIFF {
    struct TIFFDirectory tif_dir;  // first field
    void *tif_postdecode;
    void *tif_foundfield;
    struct TIFFTagMethods tif_tagmethods;
    uint32_t tif_nfieldscompat;
    void *tif_fieldscompat;
};

