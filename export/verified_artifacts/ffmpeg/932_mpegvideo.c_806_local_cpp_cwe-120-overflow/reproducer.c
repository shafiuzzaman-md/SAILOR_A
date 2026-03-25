// Combined reproducer for 932_mpegvideo.c_806_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: int16_t (auto-detected external) */
/* removed bogus int16_t stub */

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <stdint.h>

// entry from harness
extern int harness_entry(MpegEncContext *s);

int main() {
    // Allocate context
    MpegEncContext *s = (MpegEncContext *)calloc(1, sizeof(MpegEncContext));

    // Set indices/strides to simple values
    s->b8_stride = 1;
    s->block_index[0] = 0; // xy = 0

    // Allocate ac_val[0] with ONLY one 16-int16_t block (32 bytes)
    // Vulnerable memset writes 64 bytes, causing OOB
    int16_t (*ac0)[16] = (int16_t (*)[16])malloc(sizeof(int16_t[16]) * 1);
    if (!ac0) return 0;

    // Make buffer content symbolic (not strictly necessary but fine)
    klee_make_symbolic(ac0, sizeof(int16_t[16]) * 1, "ac0_buf");

    s->ac_val[0] = ac0;
    s->ac_val[1] = NULL;
    s->ac_val[2] = NULL;

    // Call entry which directly calls the vulnerable function
    harness_entry(s);
    return 0;
}
