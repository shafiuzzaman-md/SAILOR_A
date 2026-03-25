#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Mirror minimal typedefs used in harness/ftpsprop.c */
typedef int FT_Int;
typedef void* FT_Module;

typedef struct PS_DriverRec_ {
    FT_Int darken_params[8];
    FT_Int random_seed;
} *PS_Driver;

/* entry_func prototype from harness */
int ps_property_set(FT_Module module, const char* property_name, const void* value, int value_is_string);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 24) return 0;
    // Allocate PS_Driver object (module)
    PS_Driver driver = (PS_Driver)calloc(1, sizeof(*driver));

    // Property name triggers the vulnerable branch
    const char *property_name = "darkening-parameters";

    // Allocate fewer than 7 FT_Int elements so index 6 is out-of-bounds
    const size_t n = 6; // valid indices 0..5
    FT_Int *value = (FT_Int *)malloc(n * sizeof(FT_Int));
    { memcpy(value, fuzz_data + 0, 24); };

    // Non-string value path
    int value_is_string = 0;

    // Direct call to entry function
    ps_property_set((FT_Module)driver, property_name, (const void *)value, value_is_string);
    return 0;
}
