/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR -1
#endif

// Minimal type definitions required by the vulnerable code path
struct cil_symtab_datum {
    char *name;  // not used here
    char *fqn;   // used by cil_userprefixes_to_string
};

struct cil_user {
    struct cil_symtab_datum datum;
    void *bounds;       // unused
    void *roles;        // unused
    void *dftlevel;     // unused
    void *range;        // unused
    int value;          // unused
};

struct cil_userprefix {
    char *user_str;               // unused
    struct cil_user *user;        // used
    char *prefix_str;             // used
};

struct cil_list_item {
    void *data;
    struct cil_list_item *next;
};

struct cil_list {
    struct cil_list_item *head;
};

struct cil_db {
    struct cil_list *userprefixes;
};

