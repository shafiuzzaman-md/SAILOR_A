#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int sepol_port_clone(sepol_handle_t *handle, const sepol_port_t *port, sepol_port_t **out);

int main() {
    sepol_handle_t *handle = (sepol_handle_t*)calloc(1, sizeof(*handle));
    sepol_port_t *port = (sepol_port_t*)calloc(1, sizeof(*port));
    sepol_port_t *out = NULL;

    // Make struct fields symbolic, then reassign pointer fields to valid memory
    { static const unsigned char port_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(port, port_sym_data, (sizeof(*port) < sizeof(port_sym_data)) ? sizeof(*port) : sizeof(port_sym_data)); };

    // Ensure a valid non-NULL con so the vulnerable condition evaluates the field
    void *con_buf = malloc(64);
    { static const unsigned char con_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(con_buf, con_buf_data, (64 < sizeof(con_buf_data)) ? 64 : sizeof(con_buf_data)); };
    port->con = con_buf;

    // Free the sepol_port object to create a stale pointer scenario
    // This mirrors the WMI-2 pattern: read field from a freed sepol_port
    extern void sepol_port_free(sepol_port_t *port);
    sepol_port_free(port);

    // Call entry -> vulnerable function will read from the freed 'port' object
    sepol_port_clone(handle, port, &out);
    return 0;
}
