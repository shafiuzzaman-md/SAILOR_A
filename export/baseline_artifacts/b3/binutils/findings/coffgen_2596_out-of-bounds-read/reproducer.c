#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD COFF-related types to reproduce the bug. */
typedef uint64_t bfd_vma;

/* N_DEBUG sentinel (value not important for this repro as long as n_scnum != N_DEBUG). */
#define N_DEBUG -2

/* Minimal internal auxent layout used by the vulnerable code. */
union internal_auxent {
  struct {
    struct {
      struct {
        unsigned short x_lnno; /* lineno field read by the bug */
      } x_lnsz;
    } x_misc;
  } x_sym;
};

/* Minimal internal syment layout with only fields used in the code. */
struct internal_syment {
  short n_scnum;         /* section number */
  unsigned char n_numaux;/* number of aux entries */
};

/* The raw symbol/aux entry type used in the BFD COFF reader. */
typedef struct combined_entry_type {
  int is_sym; /* whether this is a symbol entry (vs aux) */
  union {
    struct internal_syment syment;
    union internal_auxent  auxent;
  } u;
} combined_entry_type;

/* Minimal COFF symbol wrapper used by the line code. */
typedef struct coff_symbol_type {
  struct {
    const char *name;
    bfd_vma value;
  } symbol;
  combined_entry_type *native; /* points into raw sym table */
} coff_symbol_type;

/* Minimal line number entry used by the code. */
struct internal_lineno {
  unsigned short line_number; /* 0 => function entry with pointer to symbol */
  union {
    bfd_vma offset; /* not used when line_number == 0 */
    void *sym;      /* points to coff_symbol_type when line_number == 0 */
  } u;
};

/* Minimal section type needed by the function. */
typedef struct asection {
  struct internal_lineno *lineno;
  size_t lineno_count;
  void *owner;
  void *used_by_bfd;
} asection;

/* Globals to mimic obj_raw_syments/obj_raw_syment_count. */
static combined_entry_type *g_raw_syms = NULL;
static size_t g_raw_syms_count = 0;

static inline combined_entry_type *obj_raw_syments(void *abfd) {
  (void)abfd;
  return g_raw_syms;
}

static inline size_t obj_raw_syment_count(void *abfd) {
  (void)abfd;
  return g_raw_syms_count;
}

/* Vulnerable function re-implemented to mirror the buggy logic. */
static void coff_find_nearest_line_with_names(
    void *abfd,
    asection *section,
    bfd_vma offset,
    const char **functionname_ptr,
    unsigned int *line_ptr)
{
  size_t i = 0;
  struct internal_lineno *l;
  unsigned int line_base = 0;
  bfd_vma last_value = 0;

  if (section->lineno != NULL) {
    l = &section->lineno[i];
    for (; i < section->lineno_count; i++) {
      if (l->line_number == 0) {
        coff_symbol_type *coff = (coff_symbol_type *)(l->u.sym);
        if (coff->symbol.value > offset)
          break;
        *functionname_ptr = coff->symbol.name;
        last_value = coff->symbol.value;
        if (coff->native) {
          combined_entry_type *s = coff->native;

          /* BFD_ASSERT(s->is_sym); */
          s = s + 1 + s->u.syment.n_numaux;

          /* In XCOFF a debugging symbol can follow the function symbol. */
          if (((size_t)((char *)s - (char *)obj_raw_syments(abfd)) <
               obj_raw_syment_count(abfd) * sizeof(*s)) &&
              s->u.syment.n_scnum == N_DEBUG)
          {
            s = s + 1 + s->u.syment.n_numaux;
          }

          /* S should now point to the .bf of the function. */
          if (((size_t)((char *)s - (char *)obj_raw_syments(abfd)) <
               obj_raw_syment_count(abfd) * sizeof(*s)) &&
              s->u.syment.n_numaux)
          {
            /* The linenumber is stored in the auxent. This is the buggy read: */
            union internal_auxent *a = &((s + 1)->u.auxent);
            line_base = a->x_sym.x_misc.x_lnsz.x_lnno;
            *line_ptr = line_base;
          }
        }
      } else {
        if (l->u.offset > offset)
          break;
        *line_ptr = l->line_number + line_base - 1;
      }
      l++;
    }

    if (i >= section->lineno_count && last_value != 0 && offset - last_value > 0x100) {
      *functionname_ptr = NULL;
      *line_ptr = 0;
    }
  }
}

int main(void) {
  /* Prepare a raw symbol table of exactly 3 entries. */
  g_raw_syms_count = 3;
  g_raw_syms = (combined_entry_type *)calloc(g_raw_syms_count, sizeof(*g_raw_syms));
  if (!g_raw_syms) {
    perror("calloc");
    return 1;
  }

  /* Layout:
   *  - coff->native will point to entry 0.
   *  - First step: s = s + 1 + s->u.syment.n_numaux.
   *    Set entry 0's n_numaux = 1, so s moves to entry 2 (last valid).
   *  - Ensure entry 2 has n_numaux != 0 to pass the check and then
   *    the code reads (s + 1)->u.auxent, which is OOB.
   */

  /* Entry 0: a symbol with one aux so we skip to entry 2. */
  g_raw_syms[0].is_sym = 1;
  g_raw_syms[0].u.syment.n_scnum = 1;     /* Not N_DEBUG */
  g_raw_syms[0].u.syment.n_numaux = 1;    /* Causes skip by +2 from entry 0 */

  /* Entry 1: contents don't matter for the repro. */
  g_raw_syms[1].is_sym = 1;
  g_raw_syms[1].u.syment.n_scnum = 1;
  g_raw_syms[1].u.syment.n_numaux = 0;

  /* Entry 2: last valid entry; set n_numaux != 0 to trigger aux read. */
  g_raw_syms[2].is_sym = 1;
  g_raw_syms[2].u.syment.n_scnum = 1;     /* Not N_DEBUG */
  g_raw_syms[2].u.syment.n_numaux = 1;    /* Nonzero so code reads (s+1)->auxent (OOB) */

  /* Set up a COFF symbol and a line number entry pointing to it. */
  coff_symbol_type coffsym;
  coffsym.symbol.name = "func";
  coffsym.symbol.value = 0;      /* <= offset so we don't break early */
  coffsym.native = &g_raw_syms[0]; /* Start from entry 0 */

  struct internal_lineno line;
  line.line_number = 0;          /* Means l->u.sym is used */
  line.u.sym = &coffsym;

  asection sec;
  memset(&sec, 0, sizeof(sec));
  sec.lineno = &line;
  sec.lineno_count = 1;
  sec.owner = &sec; /* Non-NULL owner; unused in this path */

  const char *funcname = NULL;
  unsigned int line_no = 0;
  bfd_vma offset = 0;             /* Matches coffsym.symbol.value check */

  /* abfd is opaque for our stubs. */
  void *abfd = &sec;

  /* This call triggers the out-of-bounds read at (s + 1)->u.auxent. */
  coff_find_nearest_line_with_names(abfd, &sec, offset, &funcname, &line_no);

  /* If ASan didn't abort yet, print something (unlikely if repro succeeded). */
  printf("Function: %s, Line: %u\n", funcname ? funcname : "(null)", line_no);

  free(g_raw_syms);
  return 0;
}
