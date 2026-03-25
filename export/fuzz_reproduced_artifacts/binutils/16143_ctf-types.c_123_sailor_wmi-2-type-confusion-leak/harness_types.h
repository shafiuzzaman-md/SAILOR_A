/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal neutralized harness for ctf_member_iter -> ctf_member_next */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Minimal project-local typedefs to satisfy signatures. */
typedef int ctf_id_t;               /* width not important for harness */

typedef struct ctf_dict {           /* opaque for harness */
  int _opaque;
} ctf_dict_t;

/* Forward-declare type referenced from iterator state. */
typedef struct ctf_type ctf_type_t;

/* Callback type (not used on the path; provided for prototype compatibility). */
typedef int ctf_member_f (const char *name, ctf_id_t membtype, ssize_t offset, void *arg);

/* Iterator state carrying either a dict ptr (cu) or member vector (u). */
typedef struct ctf_next {
  union {
    ctf_dict_t *ctn_fp;       /* used by vulnerable statement */
    unsigned char *ctn_cu_pad;
  } cu;
  const ctf_type_t *ctn_tp;   /* unused in harness path */
  size_t ctn_size;            /* unused in harness path */
  void (*ctn_iter_fun)(void); /* unused in harness path */
  size_t ctn_n;               /* unused in harness path */
  union {
    unsigned char *ctn_vlen;  /* mentioned in real code, unused here */
  } u;
} ctf_next_t;

