#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

// Minimal local stand-ins for required types
typedef struct { int dummy; } policydb_t;
struct cil_typeattribute { int dummy; };

int cil_typeattribute_to_policydb(policydb_t *pdb, struct cil_typeattribute *cil_attr, void *type_value_to_cil[]);

int main() {
    policydb_t *pdb = (policydb_t *)calloc(1, sizeof(policydb_t));

    struct cil_typeattribute *attr = (struct cil_typeattribute *)malloc(sizeof(struct cil_typeattribute));
    { static const unsigned char cil_attr_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(attr, cil_attr_bytes_data, (sizeof(*attr) < sizeof(cil_attr_bytes_data)) ? sizeof(*attr) : sizeof(cil_attr_bytes_data)); };

    // Create UAF: free, then pass stale pointer
    free(attr);

    void *type_value_to_cil[8] = {0};

    cil_typeattribute_to_policydb(pdb, (struct cil_typeattribute *)attr, type_value_to_cil);
    return 0;
}
