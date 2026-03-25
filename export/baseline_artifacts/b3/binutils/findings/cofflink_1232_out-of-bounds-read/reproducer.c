// Standalone C reproducer for OOB read in dores_com (bfd/cofflink.c)
// CWE-125: OOB read due to strtoul scanning past end of non-NUL-terminated buffer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal stand-ins for binutils/bfd types and helpers used by dores_com
typedef struct bfd { int dummy; } bfd;

typedef struct {
  unsigned int SizeOfHeapReserve;
  unsigned int SizeOfStackReserve;
  unsigned int SizeOfHeapCommit;
  unsigned int SizeOfStackCommit;
} pe_opthdr_t;

typedef struct {
  pe_opthdr_t pe_opthdr;
} pe_data_t;

static pe_data_t global_pe_data;

static inline pe_data_t *pe_data(bfd *abfd) {
  (void)abfd;
  return &global_pe_data;
}

// Force taking the PE code path
#define obj_pe(x) 1

// Vulnerable function adapted from bfd/cofflink.c
static char *
dores_com(char *ptr, bfd *output_bfd, int heap)
{
  if (obj_pe(output_bfd))
  {
    int val = (int)strtoul(ptr, &ptr, 0); // OOB read if ptr points to non-NUL-terminated digits

    if (heap)
      pe_data(output_bfd)->pe_opthdr.SizeOfHeapReserve = (unsigned int)val;
    else
      pe_data(output_bfd)->pe_opthdr.SizeOfStackReserve = (unsigned int)val;

    if (ptr[0] == ',') // Also OOB if strtoul advanced past end
    {
      val = (int)strtoul(ptr + 1, &ptr, 0);
      if (heap)
        pe_data(output_bfd)->pe_opthdr.SizeOfHeapCommit = (unsigned int)val;
      else
        pe_data(output_bfd)->pe_opthdr.SizeOfStackCommit = (unsigned int)val;
    }
  }
  return ptr;
}

int main(void)
{
  // Create a buffer that mimics a .drectve numeric field that reaches the end
  // of the section contents with no trailing NUL terminator.
  // ASan will place a redzone immediately after this allocation.
  size_t len = 1; // minimal size to force immediate OOB on next read
  char *buf = (char *)malloc(len);
  if (!buf) {
    perror("malloc");
    return 1;
  }

  // Fill with only digits and no NUL terminator.
  // strtoul will read the digit, then attempt to read the next char
  // to determine end-of-number, which lies in the redzone.
  buf[0] = '7';

  bfd fake_bfd;

  // Call the vulnerable parser. Under ASan, the strtoul call will trigger
  // a heap-buffer-overflow (read) when it reads past the allocated buffer.
  char *ret = dores_com(buf, &fake_bfd, 1);

  // Use results to prevent over-optimization (not necessary with -O0, but harmless).
  printf("Returned ptr=%p, HeapReserve=%u\n", (void *)ret,
         pe_data(&fake_bfd)->pe_opthdr.SizeOfHeapReserve);

  free(buf);
  return 0;
}
