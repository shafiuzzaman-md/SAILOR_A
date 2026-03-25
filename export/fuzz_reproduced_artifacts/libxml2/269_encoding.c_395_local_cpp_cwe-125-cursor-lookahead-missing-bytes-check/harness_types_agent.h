/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Minimal enum to satisfy returns */
typedef enum {
    XML_CHAR_ENCODING_NONE = 0,
    XML_CHAR_ENCODING_UTF8,
    XML_CHAR_ENCODING_UTF16LE,
    XML_CHAR_ENCODING_UTF16BE,
    XML_CHAR_ENCODING_UCS4BE,
    XML_CHAR_ENCODING_UCS4LE,
    XML_CHAR_ENCODING_EBCDIC
} xmlCharEncoding;

/* Vulnerable function (entry == vul) — keep exact vulnerable line text */
xmlCharEncoding
