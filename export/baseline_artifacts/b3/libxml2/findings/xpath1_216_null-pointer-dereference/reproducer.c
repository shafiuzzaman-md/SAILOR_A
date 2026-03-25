#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Minimal stand-ins for libxml2 types used by example/xpath1.c */
typedef char xmlChar;

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_NAMESPACE_DECL = 18
} xmlElementType;

struct _xmlNs; /* fwd */

typedef struct _xmlNs xmlNs;
typedef xmlNs* xmlNsPtr;

typedef struct _xmlNode {
    void* _private;           /* matches xmlNode layout so type is second field */
    xmlElementType type;      /* read by nodes->nodeTab[i]->type */
    const xmlChar* name;      /* used for printing */
    xmlNsPtr ns;              /* namespace of the element (unused in our path) */
} xmlNode;
typedef xmlNode* xmlNodePtr;

/* For namespace nodes libxml2 uses an xmlNs with next pointing to the owner node */
struct _xmlNs {
    xmlNsPtr next;            /* cast to xmlNodePtr in the example */
    int type;                 /* deliberately set to XML_NAMESPACE_DECL (18) */
    const xmlChar* href;      /* namespace URI */
    const xmlChar* prefix;    /* may be NULL for default namespaces -> bug trigger */
};

typedef struct _xmlNodeSet {
    int nodeNr;
    xmlNodePtr* nodeTab;
} xmlNodeSet;
typedef xmlNodeSet* xmlNodeSetPtr;

/* Vulnerable function from example/xpath1.c */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;

    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;

    fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);

        if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
            xmlNsPtr ns;

            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if(cur->ns) {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n",
                    ns->prefix, ns->href, cur->ns->href, cur->name);
            } else {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n",
                    ns->prefix, ns->href, cur->name);
            }
        } else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = nodes->nodeTab[i];    
            if(cur->ns) {
                fprintf(output, "= element node \"%s:%s\"\n",
                    cur->ns->href, cur->name);
            } else {
                fprintf(output, "= element node \"%s\"\n",
                    cur->name);
            }
        } else {
            cur = nodes->nodeTab[i];
            fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}

int main(void) {
    /* Create a fake element node that "owns" the namespace declaration */
    xmlNode* owner = (xmlNode*)calloc(1, sizeof(xmlNode));
    owner->type = XML_ELEMENT_NODE;
    owner->name = (const xmlChar*)"root";
    owner->ns = NULL; /* ensure the else-branch is taken (line 219 in source) */

    /* Create a namespace declaration with NULL prefix (default namespace) */
    xmlNs* ns = (xmlNs*)calloc(1, sizeof(xmlNs));
    ns->next = (xmlNsPtr)owner;                 /* casted back to xmlNodePtr inside */
    ns->type = XML_NAMESPACE_DECL;              /* makes the type check pass */
    ns->href = (const xmlChar*)"http://example.com/default";
    ns->prefix = NULL;                          /* BUG: passed to fprintf("%s") */

    /* Build a node set containing the namespace node (stored as xmlNodePtr) */
    xmlNodeSet* set = (xmlNodeSet*)calloc(1, sizeof(xmlNodeSet));
    set->nodeNr = 1;
    set->nodeTab = (xmlNodePtr*)calloc(1, sizeof(xmlNodePtr));
    set->nodeTab[0] = (xmlNodePtr)ns;           /* same trick used by the example */

    /* Trigger the vulnerable printing function */
    print_xpath_nodes(set, stdout);

    /* Cleanup (normally unreachable if the process crashes due to NULL %s) */
    free(set->nodeTab);
    free(set);
    free(ns);
    free(owner);
    return 0;
}
