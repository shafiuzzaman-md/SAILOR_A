#include <stdint.h>
#include <klee/klee.h>

#ifndef DCTSIZE
#define DCTSIZE 4
#endif
#ifndef DCTSTRIDE
#define DCTSTRIDE DCTSIZE
#endif
#ifndef DCTSIZE2
#define DCTSIZE2 64
#endif

typedef int16_t DCTELEM;
typedef DCTELEM DCTBLOCK[DCTSIZE2];

void ff_j_rev_dct2(DCTBLOCK data){
  int d00, d01, d10, d11;
  (void)d00; (void)d01; (void)d10; (void)d11;
  
  // Vulnerable statement copied verbatim from jrevdct.c:1143
  data[0] += 4;
  // Universal sink assertion placed AFTER the vulnerable statement
  klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Pass-through entry with no guards
int entry_point(DCTBLOCK data) {
    ff_j_rev_dct2(data);
    return 0;
}
