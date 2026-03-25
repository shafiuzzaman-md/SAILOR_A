// NO_HARNESS_TYPES
#include <stdlib.h>

char *strdup(const char *s) {
    (void)s;
    return NULL; // force tmp_name == NULL to take ERR path
}
