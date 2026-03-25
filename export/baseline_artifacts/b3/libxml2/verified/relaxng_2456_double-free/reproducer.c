// Standalone C reproducer for double-free in xmlRelaxNGSchemaFacetCheck
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal types/macros to mimic libxml2 pieces used by relaxng.c
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

// Facet types (subset, just to drive the branch logic)
#define XML_SCHEMA_FACET_MININCLUSIVE   1
#define XML_SCHEMA_FACET_MINEXCLUSIVE   2
#define XML_SCHEMA_FACET_MAXINCLUSIVE   3
#define XML_SCHEMA_FACET_MAXEXCLUSIVE   4
#define XML_SCHEMA_FACET_TOTALDIGITS    5
#define XML_SCHEMA_FACET_FRACTIONDIGITS 6
#define XML_SCHEMA_FACET_PATTERN        7
#define XML_SCHEMA_FACET_ENUMERATION    8
#define XML_SCHEMA_FACET_WHITESPACE     9
#define XML_SCHEMA_FACET_LENGTH         10
#define XML_SCHEMA_FACET_MAXLENGTH      11
#define XML_SCHEMA_FACET_MINLENGTH      12

// Minimal facet structure
typedef struct _xmlSchemaFacet {
    int type;
    void *value; // This will be set to caller-provided 'val'
} xmlSchemaFacet;

// Minimal helpers to simulate libxml2 APIs used by the buggy function
static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL) return 0;
    return strcmp((const char *)a, (const char *)b) == 0;
}

static void *xmlSchemaGetPredefinedType(const xmlChar *type, const xmlChar *ns) {
    (void)type; (void)ns;
    // Return non-NULL to drive success path
    return (void *)0x1;
}

static xmlSchemaFacet *xmlSchemaNewFacet(void) {
    xmlSchemaFacet *f = (xmlSchemaFacet *)calloc(1, sizeof(xmlSchemaFacet));
    return f;
}

static void xmlSchemaFreeFacet(xmlSchemaFacet *facet) {
    if (facet == NULL) return;
    // Bug relevance: this frees facet->value even though the value is owned by caller
    if (facet->value != NULL) {
        free(facet->value);
    }
    free(facet);
}

static int xmlSchemaCheckFacet(xmlSchemaFacet *facet, void *typ, void *unused, const xmlChar *type) {
    (void)facet; (void)typ; (void)unused; (void)type;
    // Return success to continue to success path where facet is freed
    return 0;
}

static int xmlSchemaValidateFacet(void *typ, xmlSchemaFacet *facet, const xmlChar *strval, void **value) {
    (void)typ; (void)facet; (void)strval; (void)value;
    // Return success; the function will free the facet unconditionally on success path
    return 0;
}

// Reimplementation of the vulnerable function around lines 2416-2467
static int xmlRelaxNGSchemaFacetCheck(const xmlChar *type,
                                      const xmlChar *facetname,
                                      xmlChar *val,
                                      const xmlChar *strval,
                                      void **value) {
    void *typ;
    xmlSchemaFacet *facet;
    int ret;

    if ((type == NULL) || (strval == NULL))
        return -1;

    typ = xmlSchemaGetPredefinedType(type, BAD_CAST "http://www.w3.org/2001/XMLSchema");
    if (typ == NULL)
        return -1;

    facet = xmlSchemaNewFacet();
    if (facet == NULL)
        return -1;

    if (xmlStrEqual(facetname, BAD_CAST "minInclusive")) {
        facet->type = XML_SCHEMA_FACET_MININCLUSIVE;
    } else if (xmlStrEqual(facetname, BAD_CAST "minExclusive")) {
        facet->type = XML_SCHEMA_FACET_MINEXCLUSIVE;
    } else if (xmlStrEqual(facetname, BAD_CAST "maxInclusive")) {
        facet->type = XML_SCHEMA_FACET_MAXINCLUSIVE;
    } else if (xmlStrEqual(facetname, BAD_CAST "maxExclusive")) {
        facet->type = XML_SCHEMA_FACET_MAXEXCLUSIVE;
    } else if (xmlStrEqual(facetname, BAD_CAST "totalDigits")) {
        facet->type = XML_SCHEMA_FACET_TOTALDIGITS;
    } else if (xmlStrEqual(facetname, BAD_CAST "fractionDigits")) {
        facet->type = XML_SCHEMA_FACET_FRACTIONDIGITS;
    } else if (xmlStrEqual(facetname, BAD_CAST "pattern")) {
        facet->type = XML_SCHEMA_FACET_PATTERN;
    } else if (xmlStrEqual(facetname, BAD_CAST "enumeration")) {
        facet->type = XML_SCHEMA_FACET_ENUMERATION;
    } else if (xmlStrEqual(facetname, BAD_CAST "whiteSpace")) {
        facet->type = XML_SCHEMA_FACET_WHITESPACE;
    } else if (xmlStrEqual(facetname, BAD_CAST "length")) {
        facet->type = XML_SCHEMA_FACET_LENGTH;
    } else if (xmlStrEqual(facetname, BAD_CAST "maxLength")) {
        facet->type = XML_SCHEMA_FACET_MAXLENGTH;
    } else if (xmlStrEqual(facetname, BAD_CAST "minLength")) {
        facet->type = XML_SCHEMA_FACET_MINLENGTH;
    } else {
        xmlSchemaFreeFacet(facet);
        return -1;
    }

    // Vulnerable assignment: takes ownership of caller-provided pointer
    facet->value = val;

    ret = xmlSchemaCheckFacet(facet, typ, NULL, type);
    if (ret != 0) {
        xmlSchemaFreeFacet(facet); // Frees facet->value (caller-owned)
        return -1;
    }

    ret = xmlSchemaValidateFacet(typ, facet, strval, value);

    // Unconditional free on success path as well; also frees facet->value
    xmlSchemaFreeFacet(facet);

    if (ret != 0)
        return -1;
    return 0;
}

int main(void) {
    // Simulate caller-owned value buffer
    char *caller_owned_val = (char *)malloc(16);
    if (!caller_owned_val) {
        perror("malloc");
        return 1;
    }
    strcpy(caller_owned_val, "123");

    // Inputs to drive the function down the success path
    const xmlChar *type      = BAD_CAST "integer";       // any non-NULL
    const xmlChar *facetname = BAD_CAST "minInclusive";  // recognized facet
    const xmlChar *strval    = BAD_CAST "5";             // any non-NULL
    void *outValue = NULL;

    // This call will free caller_owned_val via xmlSchemaFreeFacet(facet)
    int ret = xmlRelaxNGSchemaFacetCheck(type, facetname,
                                         (xmlChar *)caller_owned_val,
                                         strval, &outValue);

    printf("xmlRelaxNGSchemaFacetCheck returned %d\n", ret);

    // Double-free: caller still thinks it owns the memory and frees it again.
    // ASan should report double-free here, because it was already freed inside
    // xmlRelaxNGSchemaFacetCheck via xmlSchemaFreeFacet(facet).
    free(caller_owned_val);

    return 0;
}