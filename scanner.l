%{
#include "parser.tab.h"   // Token definitions from Yacc/Bison
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int line = 1, column = 1;
int token_start_column;

void update_position(char *yytext);
%}

%option noyywrap

%%
"\xEF\xBB\xBF"       { /* Skip UTF-8 BOM */ }

[ \t]+               { update_position(yytext); }
\r\n               { line++; column = 1; }
\r                 { line++; column = 1; }
\n                 { line++; column = 1; }


"{"                  { token_start_column = column; column++; return LBRACE; }
"}"                  { token_start_column = column; column++; return RBRACE; }
"["                  { token_start_column = column; column++; return LBRACKET; }
"]"                  { token_start_column = column; column++; return RBRACKET; }
":"                  { token_start_column = column; column++; return COLON; }
","                  { token_start_column = column; column++; return COMMA; }

"true"               { token_start_column = column; column += yyleng; return TRUE; }
"false"              { token_start_column = column; column += yyleng; return FALSE; }
"null"               { token_start_column = column; column += yyleng; return NULL_TOK; }

\"([^\\\"]|\\["\\/bfnrt]|\\u[0-9a-fA-F]{4})*\" {
    token_start_column = column;
    yylval.strval = strdup(yytext);
    column += yyleng;
    return STRING;
}

-?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)? {
    token_start_column = column;
    yylval.strval = strdup(yytext);
    column += yyleng;
    return NUMBER;
}

. {
    unsigned char bad = *yytext;
    if (bad >= 32 && bad <= 126) {
        fprintf(stderr, "Lexical error at line %d, column %d: unexpected character '%c'\n", line, column, bad);
    } else {
        fprintf(stderr, "Lexical error at line %d, column %d: unexpected byte 0x%02X\n", line, column, bad);
    }
    exit(1);
}

%%

void update_position(char *yytext) {
    column += strlen(yytext);
}
