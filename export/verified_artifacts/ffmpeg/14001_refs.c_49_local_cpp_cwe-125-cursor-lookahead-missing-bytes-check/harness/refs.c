#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal type shapes to reach the vulnerable access
typedef struct HEVCSPS {
    int log2_ctb_size;
    int ctb_width;
} HEVCSPS;

typedef struct HEVCPPS {
    int *ctb_addr_rs_to_ts;  // array of TS addresses
} HEVCPPS;

typedef struct RefPicList { int dummy; } RefPicList;

typedef struct RPLTabEntry {
    RefPicList refPicList[2];
} RPLTabEntry;

typedef struct HEVCFrame {
    const HEVCSPS *sps;
    const HEVCPPS *pps;
    RPLTabEntry  **rpl_tab;   // table indexed by ctb_addr_ts
    RefPicList    *refPicList; // not used on this path but present in real code
} HEVCFrame;

// Vulnerable function — keep the vulnerable statement verbatim
const RefPicList *ff_hevc_get_ref_list(const HEVCFrame *ref, int x0, int y0)
{
    const HEVCSPS *sps = ref->sps;
    int x_cb = x0 >> sps->log2_ctb_size;
    int y_cb = y0 >> sps->log2_ctb_size;
    int pic_width_cb = sps->ctb_width;
    int ctb_addr_ts  = ref->pps->ctb_addr_rs_to_ts[y_cb * pic_width_cb + x_cb];
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return &ref->rpl_tab[ctb_addr_ts]->refPicList[0];
}

// ENTRY — mandatory direct pass-through (no guards)
int entry_func(HEVCFrame *ref, int x0, int y0) {
    ff_hevc_get_ref_list(ref, x0, y0);
    return 0;
}
