/* minimal sliced harness for cavs.c:733 */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <klee/klee.h>

#ifndef NOT_AVAIL
#define NOT_AVAIL (-1)
#endif

/* Minimal AVSContext definition containing only what we touch here */
typedef struct AVSContext {
    int pred_mode_Y[3*3]; /* indices 0..8 */
} AVSContext;

/* Vulnerable function (neutralized) keeping ONLY the target statement */
int ff_cavs_init_pic(AVSContext *h)
{
    /* EXACT vulnerable statement from cavs.c:733 */
    h->pred_mode_Y[3] = h->pred_mode_Y[6] = NOT_AVAIL;
    /* Universal sink assertion placed AFTER the vulnerable statement */
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}
