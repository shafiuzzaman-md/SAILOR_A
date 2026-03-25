// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

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

int main() {
    int max_pixsteps[4];
    int max_pixstep_comps[4];

    AVPixFmtDescriptor *pix = (AVPixFmtDescriptor *)calloc(1, sizeof(AVPixFmtDescriptor));

    // Make the entire descriptor symbolic (subobject make_symbolic is not allowed)
    klee_make_symbolic(pix, sizeof(*pix), "pixdesc");

    int bad_idx = 0;
    int bad_plane;
    klee_make_symbolic(&bad_plane, sizeof(bad_plane), "bad_plane");
    klee_assume(bad_plane < 0 || bad_plane >= 4);
    pix->comp[bad_idx].plane = bad_plane;

    int pos_step;
    klee_make_symbolic(&pos_step, sizeof(pos_step), "pos_step");
    klee_assume(pos_step > 0);
    pix->comp[bad_idx].step = pos_step;

    entry_func(max_pixsteps, max_pixstep_comps, pix);
    return 0;
}
