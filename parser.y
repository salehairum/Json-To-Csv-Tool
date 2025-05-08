%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yyparse();
void yyerror(const char* s);

extern int line, column;
%}

%union {
    char* strval;
}

%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA
%token <strval> STRING NUMBER
%token TRUE FALSE NULL_TOK

%start json_text

%%

// Root of JSON
json_text:
      value                { printf("Valid JSON\n"); }
    ;

// Value can be object, array, string, etc.
value:
      object
    | array
    | STRING
    | NUMBER
    | TRUE
    | FALSE
    | NULL_TOK
    ;

// Object: { members }
object:
      LBRACE RBRACE
    | LBRACE members RBRACE
    ;

// members: key-value pairs separated by commas
members:
      pair
    | members COMMA pair
    ;

// A key-value pair
pair:
      STRING COLON value
    ;

// Array: [ elements ]
array:
      LBRACKET RBRACKET
    | LBRACKET elements RBRACKET
    ;

// elements: comma-separated values
elements:
      value
    | elements COMMA value
    ;

%%

// Error handler
extern int token_start_column;

void yyerror(const char* s) {
    fprintf(stderr, "Syntax error at line %d, column %d: %s\n", line, token_start_column, s);
}


int main() {
    if (yyparse() == 0) {
        // Parsing succeeded
        return 0;
    } else {
        // Parsing failed
        return 1;
    }
}
