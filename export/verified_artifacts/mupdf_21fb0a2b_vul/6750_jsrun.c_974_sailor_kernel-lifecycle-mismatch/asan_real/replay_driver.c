#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef NAME_BUF_SZ
#define NAME_BUF_SZ 32
#endif

int js_delglobal(js_State *J, const char *name);

int main() {
    // Allocate state and global object concretely
    js_State *J = (js_State *)calloc(1, sizeof(js_State));
    js_Object *G = (js_Object *)calloc(1, sizeof(js_Object));

    // Make contents symbolic (addresses remain concrete)
    { static const unsigned char G_obj_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(G, G_obj_bytes_data, (sizeof(*G) < sizeof(G_obj_bytes_data)) ? sizeof(*G) : sizeof(G_obj_bytes_data)); };

    // Link and then free to create a dangling pointer (UAF setup)
    J->G = G;
    free(G); // J->G is now a stale pointer

    // Prepare name buffer (symbolic content, NUL-terminated)
    char *name = (char *)calloc(NAME_BUF_SZ, 1);
    { static const unsigned char name_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name, name_buf_data, (NAME_BUF_SZ < sizeof(name_buf_data)) ? NAME_BUF_SZ : sizeof(name_buf_data)); };
    name[NAME_BUF_SZ - 1] = '\0';

    // Directly call entry to hit the vulnerable statement
    js_delglobal(J, (const char *)name);
    return 0;
}
