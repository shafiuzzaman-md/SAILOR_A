// NO_HARNESS_TYPES
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* Mirror minimal types used in harness/vc1_pred.c */
typedef struct MpegEncContext {
    int *block_index;
    int mb_x, mb_stride, mb_width, first_slice_line;
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s;
} VC1Context;

/* entry_func is defined in harness/vc1_pred.c */
int entry_func(VC1Context *v);

int main() {
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));
    if (!v) return 0;

    // Allocate too-small block_index (size 2) so access to [2] is OOB
    int *bi = (int *)malloc(2 * sizeof(int));
    if (!bi) return 0;
    klee_make_symbolic(bi, 2 * sizeof(int), "block_index_contents");

    v->s.block_index = bi;
    v->s.mb_x = 0;
    v->s.mb_stride = 0;
    v->s.mb_width = 0;
    v->s.first_slice_line = 0;

    entry_func(v);
    return 0;
}
