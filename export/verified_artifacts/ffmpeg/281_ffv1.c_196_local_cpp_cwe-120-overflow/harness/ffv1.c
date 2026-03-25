#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef MAX_QUANT_TABLES
#define MAX_QUANT_TABLES 8
#endif
#ifndef CONTEXT_SIZE
#define CONTEXT_SIZE 32
#endif
#ifndef AC_GOLOMB_RICE
#define AC_GOLOMB_RICE 0
#endif

// Minimal structs for the vulnerable path
typedef struct PlaneContext {
    int quant_table_index;
    int context_count;
    uint8_t *state;
    struct { int drift, error_sum, bias, count; } *vlc_state; // neutralized
} PlaneContext;

typedef struct FFV1SliceContext {
    PlaneContext *plane; // array of plane_count elements
} FFV1SliceContext;

typedef struct FFV1Context {
    int plane_count;
    int ac;                              // 1=range coder <-> 0=golomb rice
    uint8_t *initial_states[MAX_QUANT_TABLES];
} FFV1Context;

// ENTRY: direct pass-through to vulnerable function
void ff_ffv1_clear_slice_state(const FFV1Context *f, FFV1SliceContext *sc);
int ffv1_entry(FFV1Context *f, FFV1SliceContext *sc) {
    ff_ffv1_clear_slice_state(f, sc);
    return 0;
}

// VULNERABLE FUNCTION (keep only memcpy path)
void ff_ffv1_clear_slice_state(const FFV1Context *f, FFV1SliceContext *sc)
{
    int i;

    for (i = 0; i < f->plane_count; i++) {
        PlaneContext *p = &sc->plane[i];

        if (f->ac != AC_GOLOMB_RICE) {
            if (f->initial_states[p->quant_table_index]) {
                memcpy(p->state, f->initial_states[p->quant_table_index],
                       CONTEXT_SIZE * p->context_count);
                klee_assert(0 && "SAILOR_SINK_REACHED");
            } else {
                memset(p->state, 128, CONTEXT_SIZE * p->context_count);
            }
        } else {
            ; // neutralized
        }
    }
}
