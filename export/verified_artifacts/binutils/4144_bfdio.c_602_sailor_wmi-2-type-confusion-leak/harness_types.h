/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <sys/stat.h>

// Minimal stand-in for the real BFD type to satisfy the path
// Only fields used by bfd_get_mtime are modeled
typedef struct bfd {
    int mtime_set;
    long mtime;
} bfd;

// External dependency stubbed in stubs.c

// Vulnerable/entry function (keep verbatim vulnerable statement)
long
