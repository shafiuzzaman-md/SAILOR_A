#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <klee/klee.h>

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
#ifndef av_assert0
#define av_assert0(x) do { (void)(x); } while (0)
#endif

static void cbs_unit_uninit(CodedBitstreamUnit *unit) {
    // neutralized
    (void)unit;
}

// Vulnerable function (neutralized to keep only the path to the sink)
void ff_cbs_delete_unit(CodedBitstreamFragment *frag, int position)
{
    // Original assertion neutralized by av_assert0 macro above
    av_assert0(0 <= position && position < frag->nb_units
                             && "Unit to be deleted not in fragment.");

    cbs_unit_uninit(&frag->units[position]);

    --frag->nb_units;

    if (frag->nb_units > 0) {
        memmove(frag->units + position,
                frag->units + position + 1,
                (frag->nb_units - position) * sizeof(*frag->units));
        // Universal sink assertion: fires if memmove didn't crash
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Mandatory: simple pass-through entry with no guards
int entry_func(CodedBitstreamFragment *frag, int position) {
    ff_cbs_delete_unit(frag, position);
    return 0;
}
