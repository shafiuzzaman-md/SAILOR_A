/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type definitions to satisfy signatures */
struct bfd_hash_entry {
  struct bfd_hash_entry *next;
  const char *string;
  unsigned long hash;
};

struct bfd_hash_table {
  struct bfd_hash_entry **table;
  struct bfd_hash_entry *(*newfunc)(struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
  void *memory;
  unsigned int size;
  unsigned int count;
  unsigned int entsize;
  unsigned int frozen:1;
};

/* Vulnerable function — keep exact core logic and vulnerable statements. */
static uint32_t
