#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
// klee removed for replay

extern int sepol_node_exists(void *handle, struct policydb *policydb, unsigned int proto, void *addr, void *mask, int *response);

int main() {
    struct policydb *pdb = (struct policydb *)calloc(1, sizeof(struct policydb));
    struct ocontext *node = (struct ocontext *)calloc(1, sizeof(struct ocontext));

    // Allocate 16-byte node buffers (addr2/mask2)
    unsigned int *node_addr = (unsigned int *)malloc(4 * sizeof(unsigned int)); // 16 bytes
    unsigned int *node_mask = (unsigned int *)malloc(4 * sizeof(unsigned int)); // 16 bytes
    { static const unsigned char node_addr16_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node_addr, node_addr16_data, (4 * sizeof(unsigned int) < sizeof(node_addr16_data)) ? 4 * sizeof(unsigned int) : sizeof(node_addr16_data)); };
    { static const unsigned char node_mask16_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node_mask, node_mask16_data, (4 * sizeof(unsigned int) < sizeof(node_mask16_data)) ? 4 * sizeof(unsigned int) : sizeof(node_mask16_data)); };

    node->u.node6.addr = node_addr;
    node->u.node6.mask = node_mask;
    node->next = NULL;
    pdb->ocontexts[OCON_NODE6] = node;

    // Query buffers are too small (8 bytes)
    void *q_addr = malloc(8);
    void *q_mask = malloc(8);
    { static const unsigned char q_addr8_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(q_addr, q_addr8_data, (8 < sizeof(q_addr8_data)) ? 8 : sizeof(q_addr8_data)); };
    { static const unsigned char q_mask8_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(q_mask, q_mask8_data, (8 < sizeof(q_mask8_data)) ? 8 : sizeof(q_mask8_data)); };

    // Force the first 8 bytes to match so memcmp must read past 8
    memcpy(q_addr, node_addr, 8);
    memcpy(q_mask, node_mask, 8);

    unsigned int proto = SEPOL_PROTO_IP6;
    int response = -1;

    sepol_node_exists(NULL, pdb, proto, q_addr, q_mask, &response);
    return 0;
}
