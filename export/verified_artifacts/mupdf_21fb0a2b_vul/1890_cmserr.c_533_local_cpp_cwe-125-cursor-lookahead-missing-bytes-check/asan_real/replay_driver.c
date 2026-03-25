#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry from harness
int _cmsTagSignature2String(char *String, cmsTagSignature sig);

int main() {
    // Allocate a 4-byte buffer so that writing String[4] causes OOB
    char *buf = (char*)malloc(4);
    { static const unsigned char buf_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(buf, buf_data, (4 < sizeof(buf_data)) ? 4 : sizeof(buf_data)); };

    cmsTagSignature sig;
    { static const unsigned char sig_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&sig, sig_data, (sizeof(sig) < sizeof(sig_data)) ? sizeof(sig) : sizeof(sig_data)); };

    // Direct call to entry; no guards
    _cmsTagSignature2String(buf, sig);
    return 0;
}
