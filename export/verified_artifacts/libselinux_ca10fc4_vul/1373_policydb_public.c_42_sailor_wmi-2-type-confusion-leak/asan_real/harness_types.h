/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#ifndef PF_LEN
#define PF_LEN 1
#endif
#ifndef PF_USE_STDIO
#define PF_USE_STDIO 2
#endif
#ifndef PF_USE_MEMORY
#define PF_USE_MEMORY 3
#endif

struct policy_file {
	int type;
	FILE *fp;
	void *data;
	size_t len;
	size_t size;
	void *handle;
};

typedef struct sepol_policy_file {
	struct policy_file pf;
} sepol_policy_file_t;

