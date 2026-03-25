#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Minimal stand-in for the macro used by BFD. */
#define ISDIGIT(c) (isdigit((unsigned char)(c)))

/* Vulnerable function from bfd/coffgen.c (trimmed to be standalone). */
static inline bool
is_subsection (const char *str, const char *prefix)
{
  size_t n = strlen (prefix);
  if (strncmp (str, prefix, n) != 0)
    return false;
  if (str[n] == 0)
    return true;
  else if (str[n] != '$')
    return false;
  /* BUG: If str is too short (e.g., no byte after the digit), this reads
     one byte past the end at str[n+2]. */
  return ISDIGIT (str[n + 1]) && str[n + 2] == 0;
}

int main(void)
{
  const char *prefix = ".didat";
  size_t n = strlen(prefix);

  /* Craft a buffer that matches the prefix and has "$<digit>" after it,
     but is NOT NUL-terminated. This makes the access to str[n+2]
     a 1-byte out-of-bounds read.

     Layout: [ . d i d a t $ 5 ]  (exactly n+2 bytes, no trailing NUL)
              0 1 2 3 4 5 6 7
     Accesses inside is_subsection:
       - strncmp(str, prefix, n): reads indices [0..5] (safe)
       - str[n]   -> index 6 == '$' (safe)
       - str[n+1] -> index 7 == '5' (safe, and ISDIGIT == true)
       - str[n+2] -> index 8 (OOB read of 1 byte)
  */
  char *name = (char *)malloc(n + 2);
  if (!name) {
    perror("malloc");
    return 1;
  }
  memcpy(name, prefix, n);
  name[n] = '$';
  name[n + 1] = '5';
  /* Intentionally no NUL terminator to expose the bug. */

  /* Call the vulnerable function. AddressSanitizer should report
     an out-of-bounds read at str[n+2]. */
  volatile bool res = is_subsection(name, prefix);
  printf("is_subsection returned: %d\n", res);

  free(name);
  return 0;
}
