/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

#ifndef SEPOL_PROTO_IP6
#define SEPOL_PROTO_IP6 6
#endif
#ifndef OCON_NODE6
#define OCON_NODE6 6
#endif

struct ocontext_node6 {
    unsigned int *addr;
    unsigned int *mask;
};

struct ocontext {
    struct ocontext *next;
    union {
        struct ocontext_node6 node6;
    } u;
};

struct policydb {
    struct ocontext *ocontexts[8];
};

