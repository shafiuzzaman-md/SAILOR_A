/* AUTO-GENERATED from harness preamble */
#pragma once

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
