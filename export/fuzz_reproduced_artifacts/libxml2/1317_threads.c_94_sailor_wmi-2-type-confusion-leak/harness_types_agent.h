/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for xmlFreeMutex -> xmlCleanupMutex (threads.c:94)
 * Neutralized entry: direct pass-through to vulnerable function
 * Vulnerable statement kept verbatim, followed by universal sink assertion
 */
#include <stdlib.h>

/* Force the POSIX branch to include the vulnerable line */
#ifndef HAVE_POSIX_THREADS
#define HAVE_POSIX_THREADS 1
#endif

/* Minimal pthread types to satisfy compilation */
typedef struct { int dummy; } pthread_mutex_t;

/* Forward type from libxml2 */
typedef struct _xmlMutex {
#if HAVE_POSIX_THREADS
    pthread_mutex_t lock;
#else
    int empty;
#endif
} xmlMutex;

/* Decls to match project interface */

