/* minimal sliced harness for imgutils.c:39 */
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

/* Minimal type shims (only fields used on the path) */
typedef struct AVComponentDescriptor {
    int plane;  /* which of the 4 planes */
    int step;   /* elements between consecutive pixels */
} AVComponentDescriptor;

typedef struct AVPixFmtDescriptor {
    const char *name;
    uint8_t nb_components;
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
} AVPixFmtDescriptor;

/* Vulnerable function — keep exact vulnerable statements, neutralize the rest */
void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4],
                                const AVPixFmtDescriptor *pixdesc)
{
    int i;
    memset(max_pixsteps, 0, 4*sizeof(max_pixsteps[0]));
    if (max_pixstep_comps)
        memset(max_pixstep_comps, 0, 4*sizeof(max_pixstep_comps[0]));

    for (i = 0; i < 4; i++) {
        const AVComponentDescriptor *comp = &(pixdesc->comp[i]);
        if (comp->step > max_pixsteps[comp->plane]) {
            max_pixsteps[comp->plane] = comp->step;
            if (max_pixstep_comps)
                max_pixstep_comps[comp->plane] = i;
            /* Universal sink assertion — placed AFTER vulnerable statement */
            klee_assert(0 && "SAILOR_SINK_REACHED");
        }
    }
}

/* Pass-through entry (no guards) */
int entry_func(int max_pixsteps[4], int max_pixstep_comps[4], const AVPixFmtDescriptor *pixdesc) {
    av_image_fill_max_pixsteps(max_pixsteps, max_pixstep_comps, pixdesc);
    return 0;
}
