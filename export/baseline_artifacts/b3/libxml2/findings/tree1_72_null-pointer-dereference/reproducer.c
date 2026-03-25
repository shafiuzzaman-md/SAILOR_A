#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

int main(void) {
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;

    /* Initialize libxml2 and check for ABI mismatches */
    LIBXML_TEST_VERSION

    /* Intentionally parse a non-existent file to force xmlReadFile to fail */
    const char *path = "/no/such/file.xml";
    doc = xmlReadFile(path, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "error: could not parse file %s\n", path);
    }

    /* Vulnerable call: pass NULL doc to xmlDocGetRootElement, which will
       dereference the NULL pointer in vulnerable versions of libxml2 */
    root = xmlDocGetRootElement(doc);

    /* If the library didn't crash (e.g., if it defensively returns NULL),
       clean up and exit. */
    if (doc)
        xmlFreeDoc(doc);
    xmlCleanupParser();

    if (root)
        printf("Root element: %s\n", (const char*)root->name);

    return 0;
}
