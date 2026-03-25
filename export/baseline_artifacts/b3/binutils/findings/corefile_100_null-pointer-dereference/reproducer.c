#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal re-declarations to mirror the relevant BFD pieces. */

typedef struct bfd {
  int format; /* In real BFD this is an enum bfd_format, but int suffices here */
} bfd;

/* Enum values approximating the real ones. */
enum {
  bfd_object = 0,
  bfd_core = 1
};

enum {
  bfd_error_invalid_operation = 1,
  bfd_error_wrong_format = 2
};

/* Stubs to satisfy references in the vulnerable function's body. */
static void bfd_set_error(int error) {
  (void)error; /* No-op stub */
}

/* In real BFD this dispatches to target-specific implementations. */
#define BFD_SEND(abfd, method, args) (0)

/* Vulnerable function copied/replicated from the provided source context. */
static int bfd_core_file_pid(bfd *abfd) {
  /* NULL pointer dereference occurs here when abfd == NULL. */
  if (abfd->format != bfd_core) {
    bfd_set_error(bfd_error_invalid_operation);
    return 0;
  }
  return BFD_SEND(abfd, _core_file_pid, (abfd));
}

int main(void) {
  fprintf(stderr, "About to trigger NULL pointer dereference in bfd_core_file_pid...\n");

  /* Pass a NULL bfd pointer to directly hit the vulnerable dereference. */
  int pid = bfd_core_file_pid(NULL);

  /* This line is not expected to be reached due to the crash. */
  printf("Returned pid: %d\n", pid);
  return 0;
}
