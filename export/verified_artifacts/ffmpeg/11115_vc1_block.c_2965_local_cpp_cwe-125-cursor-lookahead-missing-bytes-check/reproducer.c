// Combined reproducer for 11115_vc1_block.c_2965_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

extern void ff_vc1_decode_blocks(VC1Context *v);

int main() {
    // Concrete allocation of the context
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));

    // Allocate destination buffers LARGE so dest is not the crashing side
    size_t dsz = 1024; // concrete
    for (int i = 0; i < 3; i++) {
        v->s.dest[i] = (uint8_t *)malloc(dsz);
        klee_make_symbolic(v->s.dest[i], dsz, "dest_buf");
    }

    // Allocate source (last_pic) SMALL so memcpy reads OOB from source
    size_t src0 = 32; // small, to force OOB when linesize*16 > 32
    size_t src1 = 32;
    size_t src2 = 32;
    v->s.last_pic.data[0] = (uint8_t *)malloc(src0);
    v->s.last_pic.data[1] = (uint8_t *)malloc(src1);
    v->s.last_pic.data[2] = (uint8_t *)malloc(src2);
    klee_make_symbolic(v->s.last_pic.data[0], src0, "last0");
    klee_make_symbolic(v->s.last_pic.data[1], src1, "last1");
    klee_make_symbolic(v->s.last_pic.data[2], src2, "last2");

    // Set fields controlling memcpy lengths
    // Make linesize symbolic and force it so linesize*16 > src0
    int linesize;
    klee_make_symbolic(&linesize, sizeof(linesize), "linesize");
    klee_assume(linesize >= 3);   // ensures linesize*16 >= 48 > 32
    klee_assume(linesize < 128);  // reasonable upper bound
    v->s.linesize = linesize;

    // mb_y = 0 so the offset term is 0 and we hit the first memcpy directly
    v->s.mb_y = 0;

    // uvlinesize not used if first memcpy crashes, but keep sane positive
    int uvlinesize;
    klee_make_symbolic(&uvlinesize, sizeof(uvlinesize), "uvlinesize");
    klee_assume(uvlinesize >= 1);
    klee_assume(uvlinesize < 128);
    v->s.uvlinesize = uvlinesize;

    // Call entry (direct pass-through to vulnerable path)
    ff_vc1_decode_blocks(v);
    return 0;
}
