#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int js_isarrayindex(js_State *J, const char *p, int *idx);

int main() {
    js_State *J = NULL; // not used by js_isarrayindex

    // Allocate a 1-byte buffer so that p[1] is out-of-bounds
    char *p = (char *)malloc(1);
    if (!p) return 0;
    { static const unsigned char p_buf_data[] = {0x30}; memcpy(p, p_buf_data, (1 < sizeof(p_buf_data)) ? 1 : sizeof(p_buf_data)); };

    // Force the path: p[0] != 0 and p[0] == '0' to hit the vulnerable line accessing p[1]
    /* klee_assume removed */

    int *idx = (int *)malloc(sizeof(int));
    if (!idx) return 0;
    { static const unsigned char idx_out_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(idx, idx_out_data, (sizeof(*idx) < sizeof(idx_out_data)) ? sizeof(*idx) : sizeof(idx_out_data)); };

    (void)js_isarrayindex(J, p, idx);
    return 0;
}
