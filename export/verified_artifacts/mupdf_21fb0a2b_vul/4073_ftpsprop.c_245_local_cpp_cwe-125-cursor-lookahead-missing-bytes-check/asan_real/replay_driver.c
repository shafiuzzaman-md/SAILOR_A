#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

/* Minimal typedefs matching harness/ftpsprop.c */
typedef int FT_Error;
typedef int FT_Int;
typedef unsigned int FT_UInt;
typedef void* FT_Module;

/* entry_func is defined in harness/ftpsprop.c */
int ps_property_get(FT_Module module, const char* property_name, void* value);

int main() {
    /* Allocate a deliberately undersized module buffer so that
       ((PS_Driver)module)->darken_params[1] may read OOB. */
    void *module_mem = malloc(sizeof(FT_Int));
    if (!module_mem) return 0;
    { static const unsigned char module_mem_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(module_mem, module_mem_data, (sizeof(FT_Int) < sizeof(module_mem_data)) ? sizeof(FT_Int) : sizeof(module_mem_data)); };

    /* Output buffer for 'value' with at least two ints for val[0], val[1]. */
    FT_Int *val_buf = (FT_Int *)malloc(2 * sizeof(FT_Int));
    if (!val_buf) return 0;
    { static const unsigned char val_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(val_buf, val_buf_data, (2 * sizeof(FT_Int) < sizeof(val_buf_data)) ? 2 * sizeof(FT_Int) : sizeof(val_buf_data)); };

    const char *prop = "darkening-parameters"; /* not used by neutralized harness */

    ps_property_get((FT_Module)module_mem, prop, (void *)val_buf);
    return 0;
}
