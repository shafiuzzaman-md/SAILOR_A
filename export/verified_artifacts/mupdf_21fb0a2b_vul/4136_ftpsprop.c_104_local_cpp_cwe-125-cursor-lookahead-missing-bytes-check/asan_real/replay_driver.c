#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int ps_property_set(FT_Module module, const char* property_name, const void* value, FT_Bool value_is_string);

int main() {
    // Allocate the driver struct
    PS_DriverRec *drv = (PS_DriverRec *)calloc(1, sizeof(PS_DriverRec));

    // Allocate TOO SMALL buffer for darken_params to trigger OOB read at index 2
    // Only 2 integers (valid indices 0 and 1); access to [2] and beyond should be OOB
    FT_Int *params = (FT_Int *)calloc(2, sizeof(FT_Int));
    { static const unsigned char darken_params_value_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(params, darken_params_value_data, (2 * sizeof(FT_Int) < sizeof(darken_params_value_data)) ? 2 * sizeof(FT_Int) : sizeof(darken_params_value_data)); };

    const char *prop = "darkening-parameters";
    FT_Bool vis = 0; // not a string

    ps_property_set((FT_Module)drv, prop, (const void *)params, vis);
    return 0;
}
