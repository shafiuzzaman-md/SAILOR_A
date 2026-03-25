/* Stub for missing glibc symbol when linking against newer-built libcrypto */
#include <stdlib.h>
long __isoc23_strtol(const char *nptr, char **endptr, int base) {
    return strtol(nptr, endptr, base);
}
