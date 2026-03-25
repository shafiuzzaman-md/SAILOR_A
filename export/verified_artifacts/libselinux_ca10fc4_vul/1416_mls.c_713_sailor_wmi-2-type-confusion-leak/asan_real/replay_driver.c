// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Must match harness/mls.c typedefs exactly
typedef struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct { int dummy; } policydb_t;

typedef struct {
    policydb_t p;
    int process_class;
} sepol_policydb_t;

typedef struct {
    int range;
} context_struct_t;

extern int sepol_mls_check(sepol_handle_t *handle, const sepol_policydb_t *policydb, const char *mls);

static void test_msg_cb(void *arg, const char *fmt, ...) {
    (void)arg; (void)fmt;
}

int main() {
    // Allocate a handle and set callback so ERR() will dereference fields
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    handle->msg_callback = test_msg_cb;
    handle->msg_callback_arg = malloc(8);
    { static const unsigned char cb_arg_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle->msg_callback_arg, cb_arg_data, (8 < sizeof(cb_arg_data)) ? 8 : sizeof(cb_arg_data)); };

    // Free to create UAF; ERR() in sepol_mls_check will touch freed memory
    free(handle);

    // Encourage same-chunk reuse to simulate type confusion reclaim
    void *reclaim = malloc(sizeof(sepol_handle_t));
    { static const unsigned char reclaim_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(reclaim, reclaim_bytes_data, (sizeof(sepol_handle_t) < sizeof(reclaim_bytes_data)) ? sizeof(sepol_handle_t) : sizeof(reclaim_bytes_data)); };

    sepol_policydb_t *policydb = (sepol_policydb_t *)calloc(1, sizeof(sepol_policydb_t));
    const char *mls = "x";

    // Pass the stale pointer
    sepol_mls_check((sepol_handle_t *)handle, policydb, mls);
    return 0;
}
