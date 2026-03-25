/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

// Minimal local types to reach the sink
typedef struct CodedBitstreamUnit {
    int type;
    uint8_t *data;
    size_t data_size;
    void *data_ref;
} CodedBitstreamUnit;

typedef struct CodedBitstreamFragment {
    CodedBitstreamUnit *units;
    int nb_units;
} CodedBitstreamFragment;

// Neutralize project-specific assert and helpers
