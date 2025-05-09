#ifndef AST_H
#define AST_H

typedef enum {
    AST_OBJECT,
    AST_ARRAY,
    AST_PAIR,
    AST_STRING,
    AST_NUMBER,
    AST_TRUE,
    AST_FALSE,
    AST_NULL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char* key;                    // For key in object pair
    char* strval;                 // For strings/numbers
    struct ASTNode* value;       // For PAIR: points to value node
    struct ASTNode* next;        // For linked lists (arrays, object pairs)
    struct ASTNode* children;    // For object/array children
} ASTNode;

ASTNode* create_node(ASTNodeType type);
ASTNode* create_pair(char* key, ASTNode* value);
ASTNode* create_value_node(ASTNodeType type, char* strval);
void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

#endif
