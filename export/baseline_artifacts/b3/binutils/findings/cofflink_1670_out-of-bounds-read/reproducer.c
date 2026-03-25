#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal re-declarations to model the vulnerable path in bfd/cofflink.c */

#define SYMNMLEN 8

typedef unsigned char bfd_byte;

typedef struct bfd_mock {
  bfd_byte *symtab;      /* Pointer to external symbol table bytes */
  size_t    symtab_size; /* Size of the symbol table in bytes */
} bfd;

/* Internal COFF symbol and aux structures (minimal fields only). */
struct internal_syment {
  uint16_t n_type;     /* Symbol type */
  uint8_t  n_sclass;   /* Storage class */
  uint8_t  n_numaux;   /* Number of aux entries */
};

union internal_auxent {
  struct {
    struct {
      struct {
        struct {
          uint32_t u32;
        } x_endndx;
      } x_fcn;
    } x_fcnary;
  } x_sym;
};

/* Stub for _bfd_coff_internal_syment_name: return an empty name */
static const char * _bfd_coff_internal_syment_name(bfd *abfd, const struct internal_syment *isym, char *buf) {
  (void)abfd; (void)isym; buf[0] = '\0'; return buf;
}

/* Vulnerable helper: reads an aux entry starting at src without bounds checks. */
static void bfd_coff_swap_aux_in(bfd *abfd, const bfd_byte *src,
                                 int type, int sclass, int indx, int numaux,
                                 union internal_auxent *dst)
{
  /* In real BFD, the external aux entry size matches the external syment size
     (commonly 18 bytes). We simulate reading that many bytes from 'src'. */
  (void)abfd; (void)type; (void)sclass; (void)indx; (void)numaux;

  unsigned char tmp[18];
  /* This memcpy will trigger ASan OOB read if 'src' points past the symtab. */
  memcpy(tmp, src, sizeof(tmp));

  /* Optionally interpret some bytes to look like an end index. */
  uint32_t val = 0;
  for (size_t i = 0; i < 4 && i < sizeof(tmp); i++)
    val |= ((uint32_t)tmp[i]) << (i * 8);
  if (dst)
    dst->x_sym.x_fcnary.x_fcn.x_endndx.u32 = val;
}

/* Minimal, self-contained reproduction of the vulnerable slice of
   coff_renumber_symbols: it unconditionally reads the aux entry at
   esym + isymesz when n_numaux == 1 without checking table bounds. */
static bool coff_renumber_symbols(bfd *input_bfd)
{
  /* Simulate having read an internal symbol that advertises one aux entry. */
  struct internal_syment isym;
  isym.n_type = 0;      /* Doesn't matter for the bug */
  isym.n_sclass = 0;    /* Doesn't matter for the bug */
  isym.n_numaux = 1;    /* Critical: triggers the aux read */

  /* External symbol entry size (typical COFF). */
  const size_t isymesz = 18;

  /* esym points to the first external symbol in the table. */
  bfd_byte *esym = input_bfd->symtab;

  /* In the true code, there should be a check that (esym + isymesz) <= end. */
  /* BUG: No bounds check before reading the aux entry. */
  union internal_auxent aux;
  bfd_coff_swap_aux_in(input_bfd, esym + isymesz,
                       isym.n_type, isym.n_sclass, 0, isym.n_numaux,
                       &aux);

  /* Do something trivial with the result to keep it live. */
  volatile uint32_t sink = aux.x_sym.x_fcnary.x_fcn.x_endndx.u32;
  (void)sink;
  return true;
}

int main(void)
{
  /* Construct a truncated/malformed symbol table: it contains exactly one
     external symbol (no space for the required aux entry). */
  const size_t isymesz = 18; /* one symbol entry, no aux follows */
  bfd abfd;
  abfd.symtab_size = isymesz; /* truncated: missing auxent bytes */
  abfd.symtab = (bfd_byte *)malloc(abfd.symtab_size);
  if (!abfd.symtab) {
    perror("malloc");
    return 1;
  }
  memset(abfd.symtab, 0x41, abfd.symtab_size); /* fill with 'A' */

  /* This call will attempt to read 18 bytes at (symtab + isymesz),
     which is just past the end of the allocated buffer, triggering
     an out-of-bounds read under ASan. */
  (void)coff_renumber_symbols(&abfd);

  free(abfd.symtab);
  return 0;
}
