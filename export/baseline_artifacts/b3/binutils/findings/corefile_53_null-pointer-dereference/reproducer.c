#include <stdio.h>
#include <stdlib.h>

/* Minimal stand-ins for BFD types and macros to make this self-contained. */
typedef struct bfd {
  int format;
} bfd;

enum bfd_format {
  bfd_object = 0,
  bfd_archive = 1,
  bfd_core = 2
};

#define bfd_error_invalid_operation 1
static void bfd_set_error(int err) { (void)err; }

/* In real BFD, BFD_SEND dispatches via the target vector; we won't reach it. */
#define BFD_SEND(abfd, field, args) ((const char *)"unreachable")

/* Vulnerable function as in bfd/corefile.c */
const char *
bfd_core_file_failing_command (bfd *abfd)
{
  if (abfd->format != bfd_core)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }
  return BFD_SEND (abfd, _core_file_failing_command, (abfd));
}

int main(void)
{
  /* Intentionally pass NULL to trigger the null-pointer dereference. */
  bfd *abfd = NULL;

  /* This call will dereference abfd->format and crash. */
  const char *cmd = bfd_core_file_failing_command(abfd);

  /* Not reached, but kept to avoid unused warnings. */
  if (cmd)
    printf("Command: %s\n", cmd);
  else
    printf("Returned NULL (unexpected if no crash)\n");

  return 0;
}
