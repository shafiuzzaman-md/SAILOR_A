/* AUTO-GENERATED from harness preamble */
#pragma once

/* Harness spine for xml-dom.c: target line 569 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Minimal type models (sliced to just what we need) */
typedef struct fz_context { int dummy; } fz_context;

struct attribute {
    char *name;
    char *value;
    struct attribute *next;
};

struct fz_xml_node_d { struct attribute *atts; };
union fz_xml_node_u { struct fz_xml_node_d d; };
struct fz_xml_node { union fz_xml_node_u u; };
union fz_xml_u { struct fz_xml_node node; };
typedef struct fz_xml { union fz_xml_u u; } fz_xml;

#ifndef FZ_TEXT_ITEM
#define FZ_TEXT_ITEM(e) (0)
#endif


/* Vulnerable function (neutralized, keep only the target path and statement) */
