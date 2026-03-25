#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Minimal replica of the vulnerable global from CMakeCCompilerId.c
// The bug is indexing this array with argc without bounds checking.
static const char info_language_extensions_default[] = "ON"; // length = 2 (plus NUL)

int main(int argc, char* argv[]) {
    // Re-exec ourselves with a large argc so that the out-of-bounds read
    // is performed using the real argc value, mirroring the original bug.
    const char* reentry = getenv("REENTRY");
    if (!reentry) {
        int N = 128; // ensure argc > strlen(info_language_extensions_default)
        char** args = (char**)malloc((N + 2) * sizeof(char*));
        if (!args) {
            perror("malloc");
            return 1;
        }
        args[0] = argv[0];
        for (int i = 1; i <= N; i++) {
            args[i] = (char*)"x"; // same dummy arg, repeated
        }
        args[N + 1] = NULL;
        setenv("REENTRY", "1", 1);
        // Use execvp to allow argv[0] to be resolved via PATH if needed.
        execvp(args[0], args);
        perror("execvp");
        free(args);
        return 1;
    }

    // Vulnerable access: mirrors `require += info_language_extensions_default[argc];`
    // in the original CMakeCCompilerId.c. With large argc, this is OOB.
    volatile int require = 0;
    require += info_language_extensions_default[argc];

    // Keep argv referenced to mirror original code pattern and avoid warnings.
    (void)argv;

    // Print something so the compiler can't optimize away `require` at -O0.
    printf("require=%d\n", require);
    return 0;
}
