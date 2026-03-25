#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD types and helpers. */
typedef unsigned char bfd_byte;
struct bfd { int dummy; };

#define SYMNMLEN 8
#define T_NULL 0
#define C_EOS 0

/* Internal COFF symbol and aux structures (only fields we need). */
struct internal_syment {
  unsigned char n_type;
  unsigned char n_sclass;
  unsigned char n_numaux;
};

union internal_auxent {
  struct {
    struct { uint32_t u32; } x_tagndx; /* We only care about this field. */
  } x_sym;
};

/* Minimal merge element structure to satisfy assignments. */
struct coff_debug_merge_element {
  char *name;
  int type;
  long tagndx;
  struct coff_debug_merge_element *next;
};

/* Global external symbol table buffer to mimic obj_coff_external_syms. */
static bfd_byte *g_external_syms = NULL;

/* Stubs mimicking the BFD helpers used in the vulnerable snippet. */
static bfd_byte *obj_coff_external_syms(struct bfd *abfd) {
  (void)abfd;
  return g_external_syms;
}

static void *bfd_alloc(struct bfd *abfd, size_t amt) {
  (void)abfd;
  return malloc(amt);
}

static void bfd_release(struct bfd *abfd, void *p) {
  (void)abfd;
  free(p);
}

static void bfd_coff_swap_sym_in(struct bfd *abfd, const bfd_byte *ext, struct internal_syment *in) {
  (void)abfd;
  /* Encode fields in the first bytes of the external entry. */
  in->n_type   = ext[0];
  in->n_sclass = ext[1];
  in->n_numaux = ext[2];
}

static const char *_bfd_coff_internal_syment_name(struct bfd *abfd, const struct internal_syment *in, char *buf) {
  (void)abfd; (void)in; (void)buf;
  return "ELEM"; /* constant, avoids extra I/O */
}

static void bfd_coff_swap_aux_in(struct bfd *abfd,
                                 const bfd_byte *ext, /* This will be esl + isymesz */
                                 int n_type, int n_sclass, int indx, int naux,
                                 union internal_auxent *dst) {
  (void)abfd; (void)n_type; (void)n_sclass; (void)indx; (void)naux;
  /* Intentionally copy 4 bytes from the provided pointer to trigger ASan
     when ext points one entry past the allocated symbol array. */
  uint32_t tmp = 0;
  memcpy(&tmp, ext, sizeof(tmp)); /* OOB read happens here. */
  dst->x_sym.x_tagndx.u32 = tmp;
}

static void trigger_vuln(void) {
  /* Choose a COFF external symbol size. Any non-zero size works for ASan. */
  const size_t isymesz = 16;            /* size of one external symbol entry */
  const size_t count   = 3;             /* total entries in external table  */

  struct bfd abfd = {0};
  struct internal_syment isyms[count];  /* internal symbols (only index 2 used) */

  bfd_byte *esym   = obj_coff_external_syms(&abfd);
  bfd_byte *esl    = esym + 2 * isymesz;          /* start at 3rd entry */
  bfd_byte *eslend = esym + count * isymesz;      /* end of table */
  struct internal_syment *islp = &isyms[2];       /* internal for 3rd entry */

  struct coff_debug_merge_element *list = NULL;
  struct coff_debug_merge_element **epp = &list;

  while (esl < eslend) {
    const char *elename;
    char elebuf[SYMNMLEN + 1];
    char *name_copy;

    bfd_coff_swap_sym_in(&abfd, esl, islp);

    *epp = (struct coff_debug_merge_element *) bfd_alloc(&abfd, sizeof(**epp));
    if (*epp == NULL) {
      fprintf(stderr, "alloc failed\n");
      exit(1);
    }

    elename = _bfd_coff_internal_syment_name(&abfd, islp, elebuf);
    name_copy = (char *) bfd_alloc(&abfd, strlen(elename) + 1);
    if (name_copy == NULL) {
      fprintf(stderr, "alloc failed\n");
      exit(1);
    }
    strcpy(name_copy, elename);

    (*epp)->name = name_copy;
    (*epp)->type = islp->n_type;
    (*epp)->tagndx = 0;

    /* Vulnerable condition: claims at least 1 aux, not T_NULL, not C_EOS. */
    if (islp->n_numaux >= 1 && islp->n_type != T_NULL && islp->n_sclass != C_EOS) {
      union internal_auxent eleaux;
      long indx;
      /* esl points at last valid symbol; esl + isymesz is one past the end. */
      bfd_coff_swap_aux_in(&abfd, (esl + isymesz), islp->n_type, islp->n_sclass, 0,
                           islp->n_numaux, &eleaux);
      indx = (long) eleaux.x_sym.x_tagndx.u32;
      /* keep indx use so the read isn't optimized away */
      if (indx == -1) puts("unreachable");
    }

    epp = &(*epp)->next;
    *epp = NULL;

    esl  += (islp->n_numaux + 1) * isymesz;
    islp +=  islp->n_numaux + 1;
  }

  /* Clean up allocated list nodes and names (avoid leaks in ASan output). */
  struct coff_debug_merge_element *cur = list;
  while (cur) {
    struct coff_debug_merge_element *next = cur->next;
    free(cur->name);
    free(cur);
    cur = next;
  }
}

int main(void) {
  const size_t isymesz = 16;
  const size_t count   = 3; /* exactly 3 symbols in the table */

  g_external_syms = (bfd_byte *) malloc(count * isymesz);
  if (!g_external_syms) {
    fprintf(stderr, "malloc failed\n");
    return 1;
  }
  memset(g_external_syms, 0, count * isymesz);

  /* Craft the last symbol (index 2) to claim it has 1 aux entry, but we
     do not provide any aux data in the buffer (table ends right after it).
     Our bfd_coff_swap_sym_in decodes:
       byte 0: n_type, byte 1: n_sclass, byte 2: n_numaux. */
  bfd_byte *last = g_external_syms + 2 * isymesz; /* 3rd entry */
  last[0] = 1; /* n_type != T_NULL */
  last[1] = 1; /* n_sclass != C_EOS */
  last[2] = 1; /* n_numaux >= 1  --> will trigger OOB read of aux */

  /* Trigger the vulnerable pattern. */
  trigger_vuln();

  free(g_external_syms);
  return 0;
}
