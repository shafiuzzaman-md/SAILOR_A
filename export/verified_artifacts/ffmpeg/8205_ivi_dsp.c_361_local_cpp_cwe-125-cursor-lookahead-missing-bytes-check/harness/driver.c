// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <klee/klee.h>
#include <stdlib.h>

// Entry prototype from harness
int ivi_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Small input to trigger OOB at in[56]
    const size_t IN_COUNT = 16; // < 57
    int32_t *inbuf = (int32_t *)calloc(IN_COUNT, sizeof(int32_t));
    klee_make_symbolic(inbuf, IN_COUNT * sizeof(int32_t), "inbuf");

    // Output buffer large enough for writes with pitch=1
    const size_t OUT_COUNT = 64; // 8 columns * 8 rows potential
    int16_t *outbuf = (int16_t *)calloc(OUT_COUNT, sizeof(int16_t));
    klee_make_symbolic(outbuf, OUT_COUNT * sizeof(int16_t), "outbuf");

    // Flags: ensure we take the branch at i==0
    uint8_t flags[8];
    klee_make_symbolic(flags, sizeof(flags), "flags");
    klee_assume(flags[0] != 0);

    ptrdiff_t pitch = 1;

    (void)ivi_entry(inbuf, outbuf, pitch, flags);
    return 0;
}
