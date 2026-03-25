// Combined reproducer for 3849_avfft.c_184_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: fn (auto-detected external) */
int fn() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

// entry from harness
int entry_func(RDFTContext *s, FFTSample *data);

// stubbed transform (defined in stubs.c)
void fake_av_tx(AVTXContext *ctx, float *dst, void *src, ptrdiff_t stride);

int main() {
    // Allocate wrapper object (RDFTContext is actually AVTXWrapper)
    AVTXWrapper *w = (AVTXWrapper *)calloc(1, sizeof(AVTXWrapper));

    // Allocate tmp with ONLY 1 float so src[1] is OOB
    const int tmp_cap = 1;
    float *tmp = (float *)malloc(tmp_cap * sizeof(float));
    klee_make_symbolic(tmp, tmp_cap * sizeof(float), "tmp_buf");

    // Make len symbolic but FORCE len == 0 to avoid memcpy overflow while triggering src[1] read
    int len;
    klee_make_symbolic(&len, sizeof(len), "len");
    klee_assume(len == 0);

    // Set up wrapper fields to reach the vulnerable block
    w->inv = 1;                // enter the if (w->inv) block containing the sink
    w->len = len;              // 0
    w->tmp = tmp;              // src points to tmp when inv != 0
    w->stride = 0;
    w->ctx = NULL;
    w->fn = fake_av_tx;        // safe no-op stub

    // Allocate data buffer; memcpy size is 0 so capacity is irrelevant, but keep >=1
    const int data_cap = 2;
    FFTSample *data = (FFTSample *)malloc(data_cap * sizeof(FFTSample));
    klee_make_symbolic(data, data_cap * sizeof(FFTSample), "data_buf");

    // Call entry (directly calls av_rdft_calc)
    entry_func((RDFTContext *)w, data);

    return 0;
}
