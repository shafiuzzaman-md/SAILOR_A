#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

extern int ft_module_get_service(FT_Module module, const char* service_id);

// Optional stub (not used on the crashing path)
static void* drv_get_interface(void* module, const char* service_id) {
    void* ret = NULL;
    memset(&ret, 0, sizeof(ret)); /* replay: no ktest data for "get_interface_ret" */;
    return ret;
}

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate library and one module
    FT_Library library = (FT_Library)calloc(1, sizeof(*library));
    FT_Module m0 = (FT_Module)calloc(1, sizeof(*m0));

    // Back-pointer
    m0->library = library;

    // Allocate modules array TOO SMALL: only 1 slot, but set num_modules = 2
    FT_Module* mods = (FT_Module*)calloc(1, sizeof(FT_Module)); // size 1
    mods[0] = m0; // first element equals `module` so first loop body is skipped

    library->modules = mods;
    library->num_modules = 2;  // deliberately larger than allocated array size

    // Class for m0 (won't be used on the crashing path)
    FT_Module_Class* cls0 = (FT_Module_Class*)calloc(1, sizeof(*cls0));
    cls0->get_interface = &drv_get_interface; // non-NULL, but we skip first body
    m0->clazz = cls0;

    // Service id buffer (symbolic)
    char sid[8];
    { memcpy(sid, fuzz_data + 0, 8); };
    sid[sizeof(sid) - 1] = '\0';

    // Call entry with m0 so that first iteration compares equal and is skipped,
    // then second iteration reads mods[1] (OOB) in `if (cur[0] != module)`
    ft_module_get_service(m0, sid);
    return 0;
}
