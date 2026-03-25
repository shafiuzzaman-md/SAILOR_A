/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for UTF16BEToUTF8 vulnerability reachability */
#include <stdint.h>
#include <stddef.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal local defs to compile */
typedef int xmlCharEncError;
#ifndef XML_ENC_ERR_SPACE
#define XML_ENC_ERR_SPACE 0
#endif
#ifndef XML_ENC_ERR_INPUT
#define XML_ENC_ERR_INPUT -1
#endif

static xmlCharEncError
UTF16BEToUTF8(void *vctxt ATTRIBUTE_UNUSED,
              unsigned char *out, int *outlen,
