// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Match harness typedefs
typedef struct sepol_handle { int _d; } sepol_handle_t;

typedef struct sepol_context {
    char *user;
    char *role;
    char *type;
    char *mls;
} sepol_context_t;

// Decls from harness
int sepol_context_clone(sepol_handle_t *handle, const sepol_context_t *con, sepol_context_t **outp);
void sepol_context_free(sepol_context_t *con);

int main() {
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_context_t *con = (sepol_context_t *)calloc(1, sizeof(sepol_context_t));

    // Ensure mls is non-NULL so the vulnerable line executes and is a valid C-string
    char *mls_buf = (char *)malloc(8);
    { static const unsigned char mls_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mls_buf, mls_buf_data, (8 < sizeof(mls_buf_data)) ? 8 : sizeof(mls_buf_data)); };
    mls_buf[7] = '\0';
    con->mls = mls_buf;

    // Free the context via real function to create a stale pointer scenario
    sepol_context_free(con);

    // Use-after-free: clone will read through 'con' and then strdup(con->mls)
    sepol_context_t *outp = NULL;
    sepol_context_clone(handle, (const sepol_context_t *)con, &outp);

    return 0;
}
