#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Entry from harness
extern int cil_gen_type_rule(struct cil_tree_node *parse_current, struct cil_tree_node *ast_node, uint32_t rule_kind);

int main() {
    // Allocate AST nodes
    struct cil_tree_node *n0 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n1 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n2 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n3 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n4 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Link next chain: n0 -> n1 -> n2 -> n3 -> n4
    n0->next = n1;
    n1->next = n2;
    n2->next = n3;
    n3->next = n4;
    n4->next = NULL;

    // Provide concrete data pointers (not dereferenced by harness, but keep valid)
    static char s0[] = "S0";
    static char s1[] = "S1";
    static char s2[] = "S2";
    static char s3[] = "S3";
    static char s4[] = "S4";
    n0->data = s0;
    n1->data = s1;
    n2->data = s2;
    n3->data = s3;  // will be read at vulnerable line
    n4->data = s4;

    // Prepare ast_node
    struct cil_tree_node *ast = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Make rule_kind symbolic to overapproximate
    uint32_t rule_kind;
    { static const unsigned char rule_kind_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&rule_kind, rule_kind_data, (sizeof(rule_kind) < sizeof(rule_kind_data)) ? sizeof(rule_kind) : sizeof(rule_kind_data)); };

    // WMI-2/UAF setup: free the node whose field is read at the vulnerable line
    // Vulnerable read: parse_current->next->next->next->data (this is n3->data)
    free(n3);

    // Call entry (pass-through to cil_gen_type_rule)
    cil_gen_type_rule(n0, ast, rule_kind);

    return 0;
}
