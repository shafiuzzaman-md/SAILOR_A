#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for BFD types used by the vulnerable code. */
typedef unsigned char bfd_byte;

typedef struct bfd_s {
  bfd_byte *external_syms;
  size_t external_syms_size;   /* Total bytes in external_syms. */
  uint32_t malicious_endndx;    /* Crafted, untrusted x_endndx. */
} bfd;

/* Minimal internal COFF symbol/aux types with only fields we touch. */
struct internal_syment {
  uint16_t n_type;
  uint8_t  n_sclass;
  uint8_t  n_numaux;
};

union internal_auxent {
  struct {
    struct {
      struct {
        struct { uint32_t u32; } x_endndx;
      } x_fcn;
    } x_fcnary;
  } x_sym;
};

/* Stubs mirroring the function names used by coff_renumber_symbols. */
static bfd_byte *obj_coff_external_syms(bfd *abfd) {
  return abfd->external_syms;
}

/* Provide a stub that returns an attacker-controlled x_endndx via abfd. */
static void bfd_coff_swap_aux_in(bfd *input_bfd,
                                 const bfd_byte *ignored,
                                 uint16_t n_type,
                                 uint8_t n_sclass,
                                 int which,
                                 uint8_t n_numaux,
                                 union internal_auxent *out) {
  (void)ignored; (void)n_type; (void)n_sclass; (void)which; (void)n_numaux;
  out->x_sym.x_fcnary.x_fcn.x_endndx.u32 = input_bfd->malicious_endndx;
}

/* Stub that intentionally reads a fixed number of bytes from 'esl'.
   When 'esl' goes out of range (due to oversized x_endndx), ASan will
   report an out-of-bounds read here, mirroring the real bug site. */
static volatile unsigned long long g_sink;
static void bfd_coff_swap_sym_in(bfd *input_bfd,
                                 const bfd_byte *esl,
                                 struct internal_syment *isym_out) {
  (void)input_bfd;
  /* Read a chunk resembling an external syment footprint.
     This will OOB once 'esl' advances past the allocated array. */
  const size_t to_read = 32; /* big enough to cross the boundary early */
  for (size_t i = 0; i < to_read; i++) {
    g_sink += esl[i]; /* ASan will catch if esl+i is out-of-bounds */
  }
  /* Populate some fields so callers can proceed if they check them. */
  isym_out->n_type = 0;
  isym_out->n_sclass = 0;
  isym_out->n_numaux = 0; /* ensure simple increment step in our loop */
}

/* A minimal reconstruction of the vulnerable portion of coff_renumber_symbols.
   It sets eslend from untrusted aux.x_endndx without validating against
   the size of obj_coff_external_syms, then iterates calling bfd_coff_swap_sym_in
   until 'esl' surpasses 'eslend'. */
static int coff_renumber_symbols(bfd *input_bfd) {
  const size_t isymesz = 4; /* symbol entry size used for stepping */

  bfd_byte *esym = obj_coff_external_syms(input_bfd);
  if (esym == NULL) return 0;

  union internal_auxent aux;
  /* Pretend to fetch aux entry; our stub pulls from input_bfd->malicious_endndx. */
  bfd_coff_swap_aux_in(input_bfd, (esym + isymesz), 0, 0, 0, 0, &aux);

  /* Setup loop bounds exactly like the vulnerable code. */
  bfd_byte *esl = esym + 2 * isymesz;
  bfd_byte *eslend = obj_coff_external_syms(input_bfd)
                   + aux.x_sym.x_fcnary.x_fcn.x_endndx.u32 * isymesz;

  struct internal_syment tmp_isym;
  while (esl < eslend) {
    /* This read becomes out-of-bounds once 'esl' walks past the allocated
       symbol array (because eslend is derived from oversized x_endndx). */
    bfd_coff_swap_sym_in(input_bfd, esl, &tmp_isym);

    /* Advance to the next symbol (ignore aux count complexities here). */
    esl += isymesz;
  }

  return 1;
}

int main(void) {
  /* Craft a tiny external symbol table. */
  const size_t isymesz = 4;     /* step size used by our loop */
  const size_t num_syms = 8;    /* actual number of external symbols */
  const size_t table_bytes = num_syms * isymesz;

  bfd *abfd = (bfd *)calloc(1, sizeof(bfd));
  if (!abfd) {
    perror("calloc bfd");
    return 1;
  }

  abfd->external_syms = (bfd_byte *)malloc(table_bytes);
  if (!abfd->external_syms) {
    perror("malloc external_syms");
    return 1;
  }
  abfd->external_syms_size = table_bytes;
  memset(abfd->external_syms, 0x41, table_bytes);

  /* Set x_endndx to a value larger than the symbol count, mimicking the
     untrusted value that the vulnerable code fails to validate. */
  abfd->malicious_endndx = (uint32_t)(num_syms + 50); /* way past end */

  fprintf(stderr,
          "Allocated %zu bytes for external symbols (%zu entries).\n",
          table_bytes, num_syms);
  fprintf(stderr,
          "Using malicious x_endndx=%u -> eslend beyond allocated table.\n",
          abfd->malicious_endndx);

  /* Trigger the vulnerable loop. ASan should report an out-of-bounds read
     originating from bfd_coff_swap_sym_in. */
  (void)coff_renumber_symbols(abfd);

  /* Cleanup (unreached if ASan aborts on error). */
  free(abfd->external_syms);
  free(abfd);
  return 0;
}
