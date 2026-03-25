#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* Minimal re-declarations to mirror the vulnerable code's expectations. */
typedef enum {
  bfd_object = 0,
  bfd_core = 1
} bfd_format;

typedef struct bfd {
  bfd_format format;
} bfd;

typedef enum {
  bfd_error_no_error = 0,
  bfd_error_invalid_operation,
  bfd_error_wrong_format
} bfd_error_type;

/* Stub for bfd_set_error used by the vulnerable function. */
void bfd_set_error(bfd_error_type err) {
  (void)err; /* no-op stub */
}

/* Stub for BFD_SEND macro used by the vulnerable function. */
#define BFD_SEND(abfd, method, args) (false)

/* Vulnerable function replicated from bfd/corefile.c
   It dereferences core_bfd->format and exec_bfd->format without NULL checks. */
bool core_file_matches_executable_p(bfd *core_bfd, bfd *exec_bfd) {
  if (core_bfd->format != bfd_core || exec_bfd->format != bfd_object) {
    bfd_set_error(bfd_error_wrong_format);
    return false;
  }
  return BFD_SEND(core_bfd, _core_file_matches_executable_p, (core_bfd, exec_bfd));
}

int main(void) {
  /* Trigger the NULL dereference by passing NULL pointers. */
  bfd *core_bfd = NULL;
  bfd *exec_bfd = NULL;

  /* This call will dereference core_bfd->format at the top of the function,
     causing a NULL pointer dereference (ASan will report it). */
  bool res = core_file_matches_executable_p(core_bfd, exec_bfd);

  /* This line will not be reached due to the crash, but keeps the compiler happy. */
  printf("Result: %d\n", (int)res);
  return 0;
}
