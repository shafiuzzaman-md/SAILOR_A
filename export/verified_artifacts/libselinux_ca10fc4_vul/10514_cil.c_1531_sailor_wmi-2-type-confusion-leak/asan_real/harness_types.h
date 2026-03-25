/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Minimal type shells to satisfy the vulnerable code
struct cil_datum { char *fqn; };
struct cil_user { struct cil_datum datum; };

struct cil_userprefix { struct cil_user *user; char *prefix_str; };

enum cil_flavor { CIL_FLAVOR_DUMMY = 0 };

struct cil_list_item {
    struct cil_list_item *next;
    enum cil_flavor flavor;
    void *data;
};

struct cil_list { struct cil_list_item *head; };

struct cil_db { struct cil_list *userprefixes; };

// Vulnerable function (neutralized) — keep signature and the vulnerable statement verbatim
