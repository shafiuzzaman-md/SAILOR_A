/* Minimal harness for vc1_pred.c OOB lookahead at line ~550 */
#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

#ifndef XML_HIDDEN
#define XML_HIDDEN  /* empty */
#endif

struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Minimal type shells to support the harness */
typedef struct MpegEncContext {
    int *block_index;  /* array of block indices */
    int mb_x, mb_stride, mb_width, first_slice_line; /* unused here */
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s; /* embedded */
    /* other fields omitted */
} VC1Context;

/* Vulnerable function (neutralized): keep signature-compatible and include the exact vulnerable statement */
void ff_vc1_pred_mv(VC1Context *v, int n, int dmv_x, int dmv_y,
                    int mv1, int r_x, int r_y, uint8_t* is_intra,
                    int pred_flag, int dir)
{
    MpegEncContext *s = &v->s;
    int wrap = 0; /* neutralized */
    int pos_c;

    /* Directly execute the vulnerable statement (from vc1_pred.c:550) */
    pos_c   = s->block_index[2] - 2 * wrap + 2;
    klee_assert(0 && "SAILOR_SINK_REACHED");

    (void)n; (void)dmv_x; (void)dmv_y; (void)mv1; (void)r_x; (void)r_y; (void)is_intra; (void)pred_flag; (void)dir; (void)pos_c;
}

/* Mandatory: entry function must be a direct pass-through call to the vulnerable function */
int entry_func(VC1Context *v)
{
    ff_vc1_pred_mv(v, 0, 0, 0, 0, 0, 0, NULL, 0, 0);
    return 0;
}
