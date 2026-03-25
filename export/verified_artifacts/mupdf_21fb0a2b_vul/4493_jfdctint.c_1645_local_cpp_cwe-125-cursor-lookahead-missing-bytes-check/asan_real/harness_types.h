/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for jpeg_fdct_13x13 */
#include <stddef.h>
#include <stdint.h>

/* Local minimal type/macro defs to match IJG libjpeg expectations */
typedef short DCTELEM;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;

#ifndef GETJSAMPLE
#define GETJSAMPLE(value) ((int) (value))
#endif

/* Vulnerable function — keep signature exact. Neutralized body keeps only the target access. */
