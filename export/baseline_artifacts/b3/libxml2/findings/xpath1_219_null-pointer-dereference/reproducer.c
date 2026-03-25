#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declarations to mirror the parts used by example/xpath1.c */

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_NAMESPACE_DECL = 18
} xmlElementType;

struct _xmlNs;  /* fwd */
struct _xmlNode; /* fwd */

typedef struct _xmlNode xmlNode;
typedef struct _xmlNs xmlNs;

typedef xmlNode* xmlNodePtr;
typedef xmlNs*   xmlNsPtr;

typedef struct _xmlNodeSet {
    int nodeNr;
    xmlNodePtr* nodeTab;
} xmlNodeSet, *xmlNodeSetPtr;

/* Layout chosen so that when an xmlNs* is cast to xmlNode*,
 * the 'type' field is found at the same offset used by the code path
 * (second field after an initial pointer-sized field). */
struct _xmlNode {
    void* _private;          /* align with real xmlNode's first field */
    xmlElementType type;     /* used by the vulnerable code */
    const char* name;        /* used by fprintf */
    xmlNode* children;       /* unused here */
    xmlNode* last;           /* unused */
    xmlNode* parent;         /* unused */
    xmlNode* next;           /* unused */
    xmlNode* prev;           /* unused */
    void*    doc;            /* unused */
    xmlNs*   ns;             /* checked by vulnerable branch */
};

struct _xmlNs {
    void* next;              /* will point to the owning node; cast to xmlNode* */
    xmlElementType type;     /* must equal XML_NAMESPACE_DECL */
    const char* href;        /* printed as %s */
    const char* prefix;      /* printed as %s; set to NULL to trigger */
    void* _private;          /* unused */
    void* context;           /* unused */
};

/* Vulnerable function copied from example/xpath1.c */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;

    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;

    fprintf(output, "Result (%d nodes):\n", size);
    for (i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);

        if (nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
            xmlNsPtr ns;

            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if (cur->ns) {
                /* Also vulnerable: ns->prefix used with %s without NULL check */
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n",
                        ns->prefix, ns->href, cur->ns->href, cur->name);
            } else {
                /* Vulnerable line (mirrors line 219 in the report) */
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n",
                        ns->prefix, ns->href, cur->name);
            }
        } else if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = nodes->nodeTab[i];
            if (cur->ns) {
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
    /* Construct a node set containing a single namespace node with a NULL prefix
     * (default namespace). This drives print_xpath_nodes into the else branch
     * that prints ns->prefix via %s without a NULL check. */

    /* The element that "owns" the namespace declaration */
    xmlNode owner = {0};
    owner.type = XML_ELEMENT_NODE;
    owner.name = "owner";
    owner.ns = NULL; /* Force the else branch at line 219 */

    /* The namespace declaration node, with a NULL prefix (default namespace) */
    xmlNs ns = {0};
    ns.type = XML_NAMESPACE_DECL;
    ns.href = "http://example.com/default"; /* non-NULL */
    ns.prefix = NULL;                        /* NULL triggers the bug */
    ns.next = &owner;                        /* as expected by the example code */

    /* Node set with one entry, pointing at the namespace node but typed as xmlNode* */
    xmlNode* tab[1];
    tab[0] = (xmlNode*)&ns; /* Intentional mixed-type storage used by the example */

    xmlNodeSet nodeset;
    nodeset.nodeNr = 1;
    nodeset.nodeTab = tab;

    /* This call should hit the vulnerable fprintf that uses a NULL %s argument */
    print_xpath_nodes(&nodeset, stdout);

    return 0;
}
