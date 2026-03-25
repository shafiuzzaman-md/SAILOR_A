#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for the pieces used around the vulnerable code. */

typedef struct {
  unsigned char *outsyms;      /* Buffer that holds swapped-out symbols/aux entries */
  size_t outsyms_size;         /* Size of outsyms buffer */
  unsigned char *linenos;      /* Unused here, but present in real code */
} flag_info_t;

/* Minimal internal auxent representation (only field we touch in the demo). */
typedef struct {
  struct {
    struct {
      struct {
        uint32_t x_lnnoptr;
      } x_fcn;
    } x_fcnary;
  } x_sym;
} internal_auxent;

/* Stub for the swapping function that writes to the location computed by the vulnerable code.
   In real BFD, this converts the internal aux entry into external format at dst. */
static void bfd_coff_swap_aux_out(void *output_bfd,
                                  const internal_auxent *ia,
                                  int n_type, int n_sclass, int idx,
                                  int n_numaux, void *dst)
{
  /* Intentionally write a sizable blob to dst to make the overflow visible to ASan. */
  (void)output_bfd; (void)ia; (void)n_type; (void)n_sclass; (void)idx; (void)n_numaux;
  unsigned char blob[64];
  memset(blob, 0xCC, sizeof(blob));
  memcpy(dst, blob, sizeof(blob));
}

/* This function models just the vulnerable portion of _bfd_coff_final_link that
   computes auxptr using (indx - syment_base + 1) * osymesz and blindly writes there. */
static void _bfd_coff_final_link(flag_info_t *flaginfo,
                                 int indx, int syment_base, size_t osymesz)
{
  /* Prepare a dummy aux entry, as done in the original code path before the write. */
  internal_auxent ia;
  ia.x_sym.x_fcnary.x_fcn.x_lnnoptr = 0xdeadbeef; /* arbitrary value */

  /* Vulnerable computation: no bounds checks on indx vs syment_base or outsyms size. */
  unsigned char *auxptr = flaginfo->outsyms
                        + ((indx - syment_base + 1) * (ptrdiff_t)osymesz);

  /* This call writes the aux entry to auxptr, which may point outside outsyms. */
  bfd_coff_swap_aux_out(NULL, &ia, 0, 0, 0, 1, auxptr);
}

int main(void)
{
  /* Allocate a small outsyms buffer (e.g., room for 4 symbols at 16 bytes each). */
  const size_t osymesz = 16;             /* size of one symbol entry (example) */
  const size_t entries = 4;               /* number of entries reserved */
  const size_t outsyms_size = entries * osymesz; /* 64 bytes */

  flag_info_t fi;
  fi.outsyms = (unsigned char *)malloc(outsyms_size);
  if (!fi.outsyms) {
    perror("malloc");
    return 1;
  }
  fi.outsyms_size = outsyms_size;
  fi.linenos = NULL;
  memset(fi.outsyms, 0x42, outsyms_size);

  /* Craft values so that the computed auxptr is well past the end of outsyms. */
  int syment_base = 0;    /* base index of symbols in output */
  int indx = 10;          /* larger than the allocated capacity implies */
  /* auxptr = outsyms + ((indx - syment_base + 1) * osymesz)
     = outsyms + (11 * 16) = outsyms + 176, well beyond 64-byte buffer. */

  fprintf(stderr, "About to trigger heap-buffer-overflow via auxptr past outsyms...\n");
  _bfd_coff_final_link(&fi, indx, syment_base, osymesz);

  /* Clean up (we likely won't reach here cleanly under ASan). */
  free(fi.outsyms);
  return 0;
}
