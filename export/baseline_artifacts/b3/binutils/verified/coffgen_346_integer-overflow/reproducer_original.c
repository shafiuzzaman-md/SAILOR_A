#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for BFD types/flags to reproduce the bug path. */
typedef uint64_t bfd_vma;
typedef uint32_t bfd_size_type; /* Force 32-bit sized arithmetic to emulate 32-bit build overflow. */
typedef unsigned int flagword;

#define HAS_RELOC  (1u<<0)
#define EXEC_P     (1u<<1)
#define HAS_LINENO (1u<<2)
#define HAS_LOCALS (1u<<3)
#define HAS_SYMS   (1u<<4)
#define D_PAGED    (1u<<5)

#define F_RELFLG 0x0001
#define F_EXEC   0x0002
#define F_LNNO   0x0004
#define F_LSYMS  0x0008

struct bfd {
  flagword flags;
  bfd_vma start_address;
  unsigned long symcount;
};

typedef struct bfd bfd;

typedef void (*bfd_cleanup)(bfd *);

struct internal_filehdr {
  unsigned int f_flags;
  unsigned int f_nsyms;
};

struct internal_aouthdr {
  bfd_vma entry;
};

struct internal_scnhdr {
  /* Size isn't important, just ensure we copy non-zero bytes during swap to
     trigger an out-of-bounds read when the pointer is invalid. */
  unsigned char bytes[32];
};

/* Globals to control behavior. */
static unsigned int g_scnhsz = 64; /* Typical COFF section header size; used to craft overflow. */

/* Stubs for external functions used by coff_real_object_p. */
static inline bfd_vma bfd_get_start_address(bfd *abfd) { return abfd->start_address; }

static void *bfd_coff_mkobject_hook(bfd *abfd, void *f, void *a) {
  (void)abfd; (void)f; (void)a;
  /* Return any non-NULL tdata so the function continues to the vulnerable path. */
  void *p = malloc(16);
  return p;
}

static unsigned int bfd_coff_scnhsz(bfd *abfd) {
  (void)abfd;
  return g_scnhsz;
}

static void *_bfd_alloc_and_read(bfd *abfd, bfd_size_type allocsize, bfd_size_type readsize) {
  (void)abfd; (void)readsize;
  /* Intentionally allocate exactly allocsize bytes. The caller has computed
     allocsize with 32-bit overflow. */
  void *p = malloc((size_t)allocsize);
  if (p) memset(p, 0x41, (size_t)allocsize);
  return p;
}

static int bfd_coff_set_arch_mach_hook(bfd *abfd, void *f) {
  (void)abfd; (void)f;
  return 1; /* Succeed so the vulnerable loop runs. */
}

static void bfd_coff_swap_scnhdr_in(bfd *abfd, void *in, void *out) {
  (void)abfd;
  /* Copy a fixed number of bytes to ensure a read occurs at the provided
     pointer. If the pointer is at or past the end of the allocated buffer,
     ASan will flag an out-of-bounds read here. */
  memcpy(out, in, sizeof(struct internal_scnhdr));
}

static int make_a_section_from_file(bfd *abfd, struct internal_scnhdr *tmp, unsigned int idx) {
  (void)abfd; (void)tmp;
  /* Let the first iteration succeed and the second fail to exit early, after
     triggering the out-of-bounds read on the second iteration. */
  return idx < 2; /* true for idx==1, false for idx==2 */
}

static void _bfd_coff_free_symbols(bfd *abfd) { (void)abfd; }

static void bfd_release(bfd *abfd, void *tdata) { (void)abfd; free(tdata); }

static void coff_object_cleanup(bfd *abfd) { (void)abfd; }

/* Vulnerable function reimplemented minimally to demonstrate the overflow and OOB read. */
static bfd_cleanup coff_real_object_p(bfd *abfd,
                                      unsigned int nscns,
                                      struct internal_filehdr *internal_f,
                                      struct internal_aouthdr *internal_a)
{
  flagword oflags = abfd->flags;
  bfd_vma ostart = bfd_get_start_address(abfd);
  void *tdata;
  bfd_size_type readsize; /* Length of section headers to read. */
  unsigned int scnhsz;
  char *external_sections;

  if (!(internal_f->f_flags & F_RELFLG))
    abfd->flags |= HAS_RELOC;
  if ((internal_f->f_flags & F_EXEC))
    abfd->flags |= EXEC_P;
  if (!(internal_f->f_flags & F_LNNO))
    abfd->flags |= HAS_LINENO;
  if (!(internal_f->f_flags & F_LSYMS))
    abfd->flags |= HAS_LOCALS;

  if ((internal_f->f_flags & F_EXEC) != 0)
    abfd->flags |= D_PAGED;

  abfd->symcount = internal_f->f_nsyms;
  if (internal_f->f_nsyms)
    abfd->flags |= HAS_SYMS;

  if (internal_a != (struct internal_aouthdr *) NULL)
    abfd->start_address = internal_a->entry;
  else
    abfd->start_address = 0;

  tdata = bfd_coff_mkobject_hook(abfd, (void *) internal_f, (void *) internal_a);
  if (tdata == NULL)
    goto fail2;

  scnhsz = bfd_coff_scnhsz(abfd);
  /* Integer overflow on 32-bit: (bfd_size_type) nscns * scnhsz. */
  readsize = (bfd_size_type) nscns * scnhsz;
  external_sections = (char *) _bfd_alloc_and_read(abfd, readsize, readsize);
  if (!external_sections)
    goto fail;

  if (nscns != 0) {
    unsigned int i;
    for (i = 0; i < nscns; i++) {
      struct internal_scnhdr tmp;
      /* Out-of-bounds read when i*scnhsz points past the tiny allocation. */
      bfd_coff_swap_scnhdr_in(abfd,
                              (void *) (external_sections + i * scnhsz),
                              (void *) &tmp);
      if (! make_a_section_from_file(abfd, &tmp, i + 1))
        goto fail;
    }
  }

  _bfd_coff_free_symbols(abfd);
  return coff_object_cleanup; /* Success path (not taken in this reproducer). */

fail:
  coff_object_cleanup(abfd);
  _bfd_coff_free_symbols(abfd);
  bfd_release(abfd, tdata);
fail2:
  abfd->flags = oflags;
  abfd->start_address = ostart;
  return NULL;
}

int main(void) {
  /* Craft values so that (uint32_t)nscns * scnhsz wraps to a small number. */
  unsigned int nscns = 0x40000001u; /* 1,073,741,825 */
  g_scnhsz = 64; /* 64-byte section headers. */
  /* 32-bit multiply: (uint32_t)0x40000001 * 64 = 64 (wraps), allocating only 64 bytes. */

  bfd abfd = {0};
  struct internal_filehdr ifh;
  memset(&ifh, 0, sizeof(ifh));
  ifh.f_flags = 0;    /* Keep flags simple so the code path proceeds. */
  ifh.f_nsyms = 0;

  /* Trigger the vulnerable code path; the second loop iteration will read
     past the 64-byte allocation by copying from external_sections + 64. */
  (void)coff_real_object_p(&abfd, nscns, &ifh, NULL);

  return 0;
}
