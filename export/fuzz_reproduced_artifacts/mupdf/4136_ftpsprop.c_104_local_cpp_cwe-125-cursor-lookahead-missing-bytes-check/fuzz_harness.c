#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int ps_property_set(FT_Module module, const char* property_name, const void* value, FT_Bool value_is_string);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate the driver struct
    PS_DriverRec *drv = (PS_DriverRec *)calloc(1, sizeof(PS_DriverRec));

    // Allocate TOO SMALL buffer for darken_params to trigger OOB read at index 2
    // Only 2 integers (valid indices 0 and 1); access to [2] and beyond should be OOB
    FT_Int *params = (FT_Int *)calloc(2, sizeof(FT_Int));
    { memcpy(params, fuzz_data + 0, 8); };

    const char *prop = "darkening-parameters";
    FT_Bool vis = 0; // not a string

    ps_property_set((FT_Module)drv, prop, (const void *)params, vis);
    return 0;
}
