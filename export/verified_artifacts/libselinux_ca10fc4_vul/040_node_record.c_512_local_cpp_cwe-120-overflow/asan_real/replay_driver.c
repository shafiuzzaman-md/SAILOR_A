// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// Minimal replicated types to match harness/node_record.c
struct sepol_handle { int dummy; };
typedef struct sepol_handle sepol_handle_t;

struct sepol_context; // opaque

struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    struct sepol_context *con;
};
typedef struct sepol_node sepol_node_t;

int sepol_node_set_mask_bytes(sepol_handle_t *handle, sepol_node_t *node, const char *mask, size_t mask_sz);

int main() {
    sepol_handle_t *handle = (sepol_handle_t*)calloc(1, sizeof(*handle));
    sepol_node_t *node = (sepol_node_t*)calloc(1, sizeof(*node));

    // Prepare existing mask so free(node->mask) is safe
    size_t old_sz = 8;
    char *old_mask = (char*)malloc(old_sz);
    { static const unsigned char old_mask_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(old_mask, old_mask_data, (old_sz < sizeof(old_mask_data)) ? old_sz : sizeof(old_mask_data)); };
    node->mask = old_mask;
    node->mask_sz = old_sz;

    // Source buffer smaller than the claimed size to expose overflow semantics
    size_t real_mask_sz = 64;
    char *mask = (char*)malloc(real_mask_sz);
    { static const unsigned char mask_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mask, mask_bytes_data, (real_mask_sz < sizeof(mask_bytes_data)) ? real_mask_sz : sizeof(mask_bytes_data)); };

    // Pass a larger size than allocated to exercise memcpy trust of mask_sz
    size_t claimed_sz = 128;

    sepol_node_set_mask_bytes(handle, node, mask, claimed_sz);
    return 0;
}
