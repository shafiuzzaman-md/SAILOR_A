// Standalone reproducer for NULL pointer dereference in bfd_core_file_failing_signal
// Compile with: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>

// Minimal re-declarations to mirror the vulnerable BFD API surface

typedef struct bfd {
    int format;  // Only the field accessed by the vulnerable function is needed
} bfd;

// Mimic the bfd error enum/tag used by bfd_set_error
enum { bfd_error_invalid_operation = 1 };

// Stub: real implementation would set a global error; here it's a no-op
static void bfd_set_error(int err) {
    (void)err;
}

// Stub: in real BFD, this dispatches to target vector methods; not needed here
#define BFD_SEND(abfd, method, args) (0)

// Mimic the bfd_format enum containing bfd_core
enum {
    bfd_core = 1
};

// Vulnerable function copied in spirit from the source context
// It dereferences abfd->format without checking abfd for NULL first
int bfd_core_file_failing_signal(bfd *abfd) {
    if (abfd->format != bfd_core) {  // NULL dereference when abfd == NULL
        bfd_set_error(bfd_error_invalid_operation);
        return 0;
    }
    return BFD_SEND(abfd, _core_file_failing_signal, (abfd));
}

int main(void) {
    // Directly pass NULL to trigger the NULL pointer dereference at abfd->format
    // AddressSanitizer will report the invalid access
    fprintf(stderr, "Triggering bfd_core_file_failing_signal(NULL) to cause NULL deref...\n");
    int sig = bfd_core_file_failing_signal(NULL);
    // The following line should be unreachable due to the crash
    printf("Returned signal: %d\n", sig);
    return 0;
}
