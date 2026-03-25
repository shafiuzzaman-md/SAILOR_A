#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate concrete _cmsICCPROFILE object
    _cmsICCPROFILE *icc = (_cmsICCPROFILE*)calloc(1, sizeof(_cmsICCPROFILE));
    if (!icc) return 0;

    // Make source ID symbolic to allow KLEE exploration
    { static const unsigned char ID8_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(icc->ProfileID.ID8, ID8_data, (sizeof(icc->ProfileID.ID8) < sizeof(ID8_data)) ? sizeof(icc->ProfileID.ID8) : sizeof(ID8_data)); };

    // Deliberately too-small destination buffer (8 < 16)
    unsigned char dest[8];
    { static const unsigned char dest_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(dest, dest_data, (sizeof(dest) < sizeof(dest_data)) ? sizeof(dest) : sizeof(dest_data)); };

    // Call the entry function (pass-through to vulnerable memmove)
    cmsGetHeaderProfileID(NULL, (cmsHPROFILE)icc, dest);
    return 0;
}
