%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern int yyparse();
void yyerror(const char* s);

extern int line, column;
%}

%union {
    char* strval;
    struct ASTNode* node;
}

%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA
%token <strval> STRING NUMBER
%token TRUE FALSE NULL_TOK
%type <node> value object array members pair elements json_text


%start json_text

%%

// Root of JSON
json_text:
      value { 
          printf("Valid JSON\n");
          extern ASTNode* ast_root;
          ast_root = $1;
      }
    ;


// Value can be object, array, string, etc.
value:
      object                       { $$ = $1; }
    | array                        { $$ = $1; }
    | STRING                       { $$ = create_value_node(AST_STRING, $1); }
    | NUMBER                       { $$ = create_value_node(AST_NUMBER, $1); }
    | TRUE                         { $$ = create_node(AST_TRUE); }
    | FALSE                        { $$ = create_node(AST_FALSE); }
    | NULL_TOK                     { $$ = create_node(AST_NULL); }
    ;


// Object: { members }
object:
      LBRACE RBRACE                      { $$ = create_node(AST_OBJECT); }
    | LBRACE members RBRACE              { $$ = create_node(AST_OBJECT); $$->children = $2; }
    ;

// members: key-value pairs separated by commas
members:
      pair                         { $$ = $1; }
    | members COMMA pair          { $$ = $1; 
                                    ASTNode* temp = $1;
                                    while (temp->next) temp = temp->next;
                                    temp->next = $3;
                                  }
    ;


// A key-value pair
pair:
      STRING COLON value                 { $$ = create_pair($1, $3); }
    ;


// Array: [ elements ]
array:
      LBRACKET RBRACKET                  { $$ = create_node(AST_ARRAY); }
    | LBRACKET elements RBRACKET         { $$ = create_node(AST_ARRAY); $$->children = $2; }
    ;


// elements: comma-separated values
elements:
      value                        { $$ = $1; }
    | elements COMMA value        { $$ = $1;
                                    ASTNode* temp = $1;
                                    while (temp->next) temp = temp->next;
                                    temp->next = $3;
                                  }
    ;

%%

ASTNode* ast_root = NULL;
int print_ast_flag = 0;  // Set by --print-ast

// Error handler
extern int token_start_column;

void yyerror(const char* s) {
    fprintf(stderr, "Syntax error at line %d, column %d: %s\n", line, token_start_column, s);
}


int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            print_ast_flag = 1;
        }
    }

    if (yyparse() == 0) {
        if (print_ast_flag && ast_root) {
            printf("\n=== AST ===\n");
            print_ast(ast_root, 0);
        }
        return 0;
    } else {
        return 1;
    }
}

