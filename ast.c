#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->key = NULL;
    node->strval = NULL;
    node->value = NULL;
    node->next = NULL;
    node->children = NULL;
    return node;
}

ASTNode* create_value_node(ASTNodeType type, char* strval) {
    ASTNode* node = create_node(type);
    node->strval = strdup(strval);
    return node;
}

ASTNode* create_pair(char* key, ASTNode* value) {
    ASTNode* node = create_node(AST_PAIR);
    node->key = strdup(key);
    node->value = value;
    return node;
}

void print_indent(int indent) {
    for (int i = 0; i < indent; ++i) printf("  ");
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    while (node) {
        print_indent(indent);
        switch (node->type) {
            case AST_OBJECT:
                printf("OBJECT {\n");
                print_ast(node->children, indent + 1);
                print_indent(indent);
                printf("}\n");
                break;
            case AST_ARRAY:
                printf("ARRAY [\n");
                print_ast(node->children, indent + 1);
                print_indent(indent);
                printf("]\n");
                break;
            case AST_PAIR:
                printf("PAIR \"%s\":\n", node->key);
                print_ast(node->value, indent + 1);
                break;
            case AST_STRING:
            case AST_NUMBER:
                printf("%s: %s\n", node->type == AST_STRING ? "STRING" : "NUMBER", node->strval);
                break;
            case AST_TRUE:
                printf("TRUE\n"); break;
            case AST_FALSE:
                printf("FALSE\n"); break;
            case AST_NULL:
                printf("NULL\n"); break;
        }
        node = node->next;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->next);
    free_ast(node->children);
    if (node->key) free(node->key);
    if (node->strval) free(node->strval);
    if (node->value) free_ast(node->value);
    free(node);
}
