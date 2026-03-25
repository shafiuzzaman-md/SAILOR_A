#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Minimal stand-ins for BFD/COFF types used by the vulnerable code. */
typedef unsigned char bfd_byte;
typedef struct bfd { int dummy; } bfd;

#define SYMNMLEN 8
#define T_NULL 0
#define C_EOS 99

/* Size of one external symbol table entry in COFF (commonly 18 bytes). */
static const size_t isymesz = 18;

/* Fake global symbol table buffer, standing in for obj_coff_external_syms(abfd). */
static bfd_byte *g_external_syms = NULL;
static size_t g_external_syms_size = 0; /* in bytes */

/* Helper to mimic obj_coff_external_syms(abfd) */
static inline bfd_byte *obj_coff_external_syms(bfd *abfd) {
  (void)abfd;
  return g_external_syms;
}

/* Minimal internal symbol entry containing only the fields used. */
struct internal_syment {
  uint16_t n_type;
  uint8_t n_sclass;
  uint8_t n_numaux;
};

/* Minimal auxent layout providing the fields used by the vulnerable code. */
union internal_auxent {
  struct {
    struct {
      struct {
        union { uint32_t u32; } x_endndx;
      } x_fcn;
    } x_fcnary;
    struct {
      union { uint32_t u32; } x_tagndx;
    } x_sym;
  } x_sym;
};

/* Stub: reads an aux entry. We'll just set the crafted end index. */
static void bfd_coff_swap_aux_in(bfd *abfd, const bfd_byte *ext,
                                 uint16_t n_type, uint8_t n_sclass,
                                 int which, uint8_t n_numaux,
                                 union internal_auxent *aux) {
  (void)abfd; (void)ext; (void)n_type; (void)n_sclass; (void)which; (void)n_numaux;
  /* Craft a bogus, very large end index so eslend points past the real table. */
  aux->x_sym.x_fcnary.x_fcn.x_endndx.u32 = 100000; /* way past real end */
}

/* Stub: unsafe symbol swap-in that copies a full external symbol record. */
static void bfd_coff_swap_sym_in(bfd *abfd, const bfd_byte *ext, struct internal_syment *in) {
  (void)abfd;
  /* Intentionally read isymesz bytes from 'ext' without bounds checks.
     This will trigger ASan once 'ext' moves past the allocated table. */
  unsigned char tmp[18];
  memcpy(tmp, ext, sizeof(tmp));
  /* Fill some fields; exact semantics aren't important for the OOB. */
  in->n_type = tmp[0];
  in->n_sclass = tmp[1];
  in->n_numaux = tmp[2];
}

/* Reproducer version of the vulnerable loop from coff_renumber_symbols. */
static bool coff_renumber_symbols(bfd *input_bfd) {
  /* Set up a fake starting point at the beginning of the symbol table. */
  bfd_byte *esym = obj_coff_external_syms(input_bfd);
  if (esym == NULL) return false;

  /* Pretend we already read the main symbol and are now reading aux to get end index. */
  union internal_auxent aux;
  struct internal_syment isym = { .n_type = 1, .n_sclass = 2, .n_numaux = 1 };
  bfd_coff_swap_aux_in(input_bfd, (esym + isymesz), isym.n_type, isym.n_sclass, 0, isym.n_numaux, &aux);

  /* Prepare iteration state like in the real function. */
  struct internal_syment isyms_buf[4];
  struct internal_syment *isymp = &isyms_buf[0];
  struct internal_syment *islp = isymp + 2; /* where swapped-in element symbol will go */

  bfd_byte *esl = esym + 2 * isymesz; /* start 2 entries in */
  bfd_byte *eslend = (obj_coff_external_syms(input_bfd)
                      + aux.x_sym.x_fcnary.x_fcn.x_endndx.u32 * isymesz);

  /* Vulnerable loop: bounds check uses eslend computed from untrusted end index. */
  while (esl < eslend) {
    /* OOB read occurs here once esl advances beyond the allocated symbol table. */
    bfd_coff_swap_sym_in(input_bfd, esl, islp);

    /* In the real code, the loop advances over each symbol (and any aux entries).
       For reproduction, advance by one external symbol size each iteration. */
    esl += isymesz;

    /* Safety to avoid infinite loop if ASan is disabled (not expected here). */
    if ((size_t)(esl - obj_coff_external_syms(input_bfd)) > g_external_syms_size + 1024) {
      break;
    }
  }

  return true;
}

int main(void) {
  /* Build a small fake external symbol table with a handful of entries. */
  const size_t entries = 4; /* real table is small */
  g_external_syms_size = entries * isymesz;
  g_external_syms = (bfd_byte *)malloc(g_external_syms_size);
  if (!g_external_syms) {
    fprintf(stderr, "alloc failed\n");
    return 1;
  }
  /* Fill with dummy data. */
  for (size_t i = 0; i < g_external_syms_size; i++) g_external_syms[i] = (unsigned char)(i & 0xFF);

  bfd fake_bfd = {0};

  /* This call will trigger an out-of-bounds read in bfd_coff_swap_sym_in
     because eslend is computed past the end of g_external_syms. */
  (void)coff_renumber_symbols(&fake_bfd);

  /* Cleanup (won't be reached if ASan aborts on OOB). */
  free(g_external_syms);
  return 0;
}
