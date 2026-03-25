#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// entry_func is defined in the harness and directly calls sepol_bool_free
int sepol_bool_free(sepol_bool_t *b);

int main() {
    // 1) Concrete struct allocation
    sepol_bool_t *b = (sepol_bool_t *)calloc(1, sizeof(sepol_bool_t));
    if (!b) return 0;

    // 2) Concrete name buffer, symbolic contents
    const size_t NAME_SZ = 32;
    char *name = (char *)malloc(NAME_SZ);
    if (!name) return 0;
    { static const unsigned char bool_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name, bool_name_data, (NAME_SZ < sizeof(bool_name_data)) ? NAME_SZ : sizeof(bool_name_data)); };
    name[NAME_SZ - 1] = '\0';

    // 3) Wire fields
    b->name = name;

    // 4) First call: frees name and the struct
    sepol_bool_free(b);

    // 5) Second call: dereference of freed struct (boolean->name) and free() again
    sepol_bool_free(b);

    return 0;
}
