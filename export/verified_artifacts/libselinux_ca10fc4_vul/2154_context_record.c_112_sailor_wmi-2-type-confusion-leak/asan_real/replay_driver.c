// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Minimal compatible types (must match harness layouts)
typedef void (*sepol_msg_callback_t)(void *arg, const char *fmt, ...);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_context {
    char *user;
    char *role;
    char *type;
    char *mls;
} sepol_context_t;

int sepol_context_set_mls(sepol_handle_t *handle, sepol_context_t *con, const char *mls);

int main() {
    // Allocate a context with a real mls buffer
    sepol_context_t *con = (sepol_context_t *)calloc(1, sizeof(sepol_context_t));
    if (!con) return 0;
    char *mls_buf = (char *)malloc(32);
    if (!mls_buf) return 0;
    { static const unsigned char mls_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mls_buf, mls_buf_data, (32 < sizeof(mls_buf_data)) ? 32 : sizeof(mls_buf_data)); };
    mls_buf[31] = '\0';
    con->mls = mls_buf;

    // Create a sepol_handle_t, make its bytes symbolic, then free it to create a stale pointer
    sepol_handle_t *h = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    if (!h) return 0;
    { static const unsigned char handle_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(h, handle_bytes_data, (sizeof(*h) < sizeof(handle_bytes_data)) ? sizeof(*h) : sizeof(handle_bytes_data)); };
    free(h);

    // Call entry with the FREED handle pointer; ERR() will read through it on OOM path
    sepol_context_set_mls(h, con, mls_buf);
    return 0;
}
