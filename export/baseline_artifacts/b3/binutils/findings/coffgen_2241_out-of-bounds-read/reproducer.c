#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Minimal stand-ins for BFD types/macros used by coff_print_symbol. */
typedef unsigned long long bfd_vma;
#define BFD_ASSERT(x) do { assert(x); } while (0)

/* Forward declare to allow self-referential pointers inside unions. */
typedef struct combined_entry_type combined_entry_type;

typedef struct lineno_cache_entry {
  int dummy;
} lineno_cache_entry;

/* Fake ABFD that carries the raw symbols array and its count. */
typedef struct {
  combined_entry_type *root;
  size_t count;
} fake_abfd;

#define obj_raw_syments(abfd)      (((fake_abfd*)(abfd))->root)
#define obj_raw_syment_count(abfd) (((fake_abfd*)(abfd))->count)

/* Some constants referenced by the switch in the original code. */
#define C_FILE 103
#define C_DWARF 112
#define C_STAT  105
#define T_NULL  0

/* Signature helper types for aux entries (only what's used). */
typedef struct {
  union { combined_entry_type *p; unsigned int u32; } x_tagndx;
} aux_x_sym;

typedef struct {
  int x_ftype;
  struct { struct { struct { unsigned long x_offset; } x_n; } x_n; } x_n;
} aux_x_file;

typedef struct {
  uint64_t x_scnlen;
  int64_t  x_nreloc;
} aux_x_sect;

typedef struct {
  unsigned long x_scnlen;
  int x_nreloc;
  int x_nlinno;
  int x_checksum;
  int x_associated;
  int x_comdat;
} aux_x_scn;

/* Minimal representation of the COFF combined entry. */
struct combined_entry_type {
  int is_sym;     /* 1 if a symbol entry, 0 if an auxiliary entry. */
  int fix_value;  /* Interpret n_value as index if set (we won't use). */
  int fix_tag;    /* Interpret aux tag as pointer if set. */
  union {
    struct { /* Symbol entry fields we touch. */
      void *n_value;           /* Used for printing address. */
      int   n_scnum;
      int   n_flags;
      int   n_type;
      int   n_sclass;
      unsigned int n_numaux;   /* Number of following aux entries. */
    } syment;
    struct { /* Auxiliary entry fields we touch. */
      aux_x_sym  x_sym;
      aux_x_file x_file;
      aux_x_sect x_sect;
      aux_x_scn  x_scn;
    } auxent;
  } u;
};

/* Minimal asymbol/COFF symbol wrappers. */
typedef struct {
  combined_entry_type *native;
  struct lineno_cache_entry *lineno;
} asymbol_coff;

typedef struct {
  const char *name;
  void *the_bfd;       /* Not used here, but typical API has it. */
  asymbol_coff *tc_data; /* COFF-specific data. */
} asymbol;

static inline asymbol_coff *coffsymbol(asymbol *sym) { return sym->tc_data; }

/* Stub: always return 0 so the switch on sclass runs. */
static int bfd_coff_print_aux(void *abfd, FILE *file,
                              combined_entry_type *root,
                              combined_entry_type *combined,
                              combined_entry_type *auxp,
                              unsigned int aux)
{
  (void)abfd; (void)file; (void)root; (void)combined; (void)auxp; (void)aux;
  return 0;
}

/* Minimal vma printer stub. */
static void bfd_fprintf_vma(void *abfd, FILE *file, bfd_vma v)
{
  (void)abfd;
  fprintf(file, "%llx", (unsigned long long)v);
}

/* Only the bfd_print_symbol_all path is implemented, mirroring the vulnerable logic. */
static void coff_print_symbol(void *abfd, FILE *file, asymbol *symbol, int how)
{
  enum { bfd_print_symbol_all = 1 };
  const char *symname = symbol->name ? symbol->name : "<no name>";

  if (how == bfd_print_symbol_all) {
    if (coffsymbol(symbol)->native) {
      bfd_vma val;
      unsigned int aux;
      combined_entry_type *combined = coffsymbol(symbol)->native;
      combined_entry_type *root = obj_raw_syments(abfd);
      struct lineno_cache_entry *l = coffsymbol(symbol)->lineno;
      (void)l; /* Unused in this reproducer. */

      fprintf(file, "[%3ld]", (long)(combined - root));

      if (combined < obj_raw_syments(abfd)
          || combined >= obj_raw_syments(abfd) + obj_raw_syment_count(abfd)) {
        fprintf(file, "<corrupt info> %s", symname);
        return;
      }

      BFD_ASSERT(combined->is_sym);
      if (!combined->fix_value)
        val = (bfd_vma)(uintptr_t)combined->u.syment.n_value;
      else
        val = (((uintptr_t) combined->u.syment.n_value - (uintptr_t) root)
               / sizeof(combined_entry_type));

      fprintf(file, "(sec %2d)(fl 0x%02x)(ty %4x)(scl %3d) (nx %d) 0x",
              combined->u.syment.n_scnum,
              combined->u.syment.n_flags,
              combined->u.syment.n_type,
              combined->u.syment.n_sclass,
              combined->u.syment.n_numaux);
      bfd_fprintf_vma(abfd, file, val);
      fprintf(file, " %s", symname);

      /* Vulnerable loop: auxp can point past end when n_numaux is oversized. */
      for (aux = 0; aux < combined->u.syment.n_numaux; aux++) {
        combined_entry_type *auxp = combined + aux + 1;
        long tagndx;

        /* Out-of-bounds read of auxp->is_sym when auxp is beyond array. */
        BFD_ASSERT(!auxp->is_sym);
        if (auxp->fix_tag)
          tagndx = (long)(auxp->u.auxent.x_sym.x_tagndx.p - root);
        else
          tagndx = auxp->u.auxent.x_sym.x_tagndx.u32;

        fprintf(file, "\n");

        if (bfd_coff_print_aux(abfd, file, root, combined, auxp, aux))
          continue;

        switch (combined->u.syment.n_sclass) {
          case C_FILE:
            fprintf(file, "File ");
            if (auxp->u.auxent.x_file.x_ftype)
              fprintf(file, "ftype %d fname \"%s\"",
                      auxp->u.auxent.x_file.x_ftype,
                      (char *)(uintptr_t)auxp->u.auxent.x_file.x_n.x_n.x_offset);
            break;
          case C_DWARF:
            fprintf(file, "AUX scnlen %#" PRIx64 " nreloc %" PRId64,
                    (uint64_t)auxp->u.auxent.x_sect.x_scnlen,
                    (int64_t)auxp->u.auxent.x_sect.x_nreloc);
            break;
          case C_STAT:
            if (combined->u.syment.n_type == T_NULL) {
              fprintf(file, "AUX scnlen 0x%lx nreloc %d nlnno %d",
                      auxp->u.auxent.x_scn.x_scnlen,
                      auxp->u.auxent.x_scn.x_nreloc,
                      auxp->u.auxent.x_scn.x_nlinno);
              if (auxp->u.auxent.x_scn.x_checksum != 0
                  || auxp->u.auxent.x_scn.x_associated != 0
                  || auxp->u.auxent.x_scn.x_comdat != 0)
                fprintf(file, " checksum 0x%x assoc %d comdat %d",
                        auxp->u.auxent.x_scn.x_checksum,
                        auxp->u.auxent.x_scn.x_associated,
                        auxp->u.auxent.x_scn.x_comdat);
            }
            break;
          default:
            break;
        }
        (void)tagndx; /* silence unused warning in some paths */
      }
    }
  }
}

int main(void)
{
  /* Allocate a raw symbols array with just 1 entry. */
  fake_abfd *abfd = (fake_abfd *)calloc(1, sizeof(*abfd));
  if (!abfd) return 1;
  abfd->count = 1;
  abfd->root = (combined_entry_type *)aligned_alloc(16, sizeof(combined_entry_type) * abfd->count);
  if (!abfd->root) return 1;

  combined_entry_type *root = abfd->root;
  memset(root, 0, sizeof(*root) * abfd->count);

  /* Prepare a single symbol at the last position with an oversized n_numaux. */
  root[0].is_sym = 1;           /* Mark as a symbol entry. */
  root[0].fix_value = 0;
  root[0].fix_tag = 0;
  root[0].u.syment.n_value = (void*)0x1234; /* Just some value to print. */
  root[0].u.syment.n_scnum = 1;
  root[0].u.syment.n_flags = 0;
  root[0].u.syment.n_type = T_NULL;
  root[0].u.syment.n_sclass = C_FILE; /* Any class; we won't get that far. */
  root[0].u.syment.n_numaux = 100;    /* Oversized: forces auxp past end on first iter. */

  /* Set up the asymbol wrapper pointing to this single combined entry. */
  asymbol_coff *csym = (asymbol_coff *)calloc(1, sizeof(*csym));
  if (!csym) return 1;
  csym->native = &root[0];
  csym->lineno = NULL;

  asymbol sym;
  sym.name = "trigger";
  sym.the_bfd = abfd;
  sym.tc_data = csym;

  /* Invoke the vulnerable path: on aux=0, auxp = combined+1 => OOB. */
  coff_print_symbol(abfd, stdout, &sym, 1 /* bfd_print_symbol_all */);

  /* Cleanup (unreached if ASan aborts on OOB). */
  free(csym);
  free(abfd->root);
  free(abfd);
  return 0;
}
