#include <stddef.h>
// Combined reproducer for 24302_imgutils.c_39_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: entry (auto-detected external) */
int entry() { return 0; }

/* PROACTIVE: shims (auto-detected external) */
int shims() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* Minimal type shims matching harness/imgutils.c */
typedef struct AVComponentDescriptor {
    int plane;
    int step;
} AVComponentDescriptor;

typedef struct AVPixFmtDescriptor {
    const char *name;
    uint8_t nb_components;
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
} AVPixFmtDescriptor;

// entry_func from harness/imgutils.c
extern int entry_func(int max_pixsteps[4], int max_pixstep_comps[4], const AVPixFmtDescriptor *pixdesc);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    int max_pixsteps[4];
    int max_pixstep_comps[4];

    AVPixFmtDescriptor *pix = (AVPixFmtDescriptor *)calloc(1, sizeof(AVPixFmtDescriptor));

    // Make the entire descriptor symbolic (subobject make_symbolic is not allowed)
    memcpy(pix, fuzz_data + (0), sizeof(*pix));

    int bad_idx = 0;
    int bad_plane;
    memcpy(&bad_plane, fuzz_data + (sizeof(*pix)), sizeof(bad_plane));
    
    pix->comp[bad_idx].plane = bad_plane;

    int pos_step;
    memcpy(&pos_step, fuzz_data + (sizeof(*pix) + sizeof(bad_plane)), sizeof(pos_step));
    
    pix->comp[bad_idx].step = pos_step;

    entry_func(max_pixsteps, max_pixstep_comps, pix);
    return 0;
}
