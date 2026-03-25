#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry from harness
int _cmsTagSignature2String(char *String, cmsTagSignature sig);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 4) return 0;
    // Allocate a 4-byte buffer so that writing String[4] causes OOB
    char *buf = (char*)malloc(4);
    { memcpy(buf, fuzz_data + 0, 4); };

    cmsTagSignature sig;
    { static const unsigned char sig_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&sig, sig_data, (sizeof(sig) < sizeof(sig_data)) ? sizeof(sig) : sizeof(sig_data)); };

    // Direct call to entry; no guards
    _cmsTagSignature2String(buf, sig);
    return 0;
}
