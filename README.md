# JSON to CSV Tool
This tool will flatten Json objects into CSV Tables.

### Commands for running
```bash
flex scanner.l        # Generates lex.yy.c
bison -d parser.y     # Generates parser.tab.c and parser.tab.h
gcc -o json parser.tab.c lex.yy.c ast.c schema.c semantics.c -lfl
./json < input.json --print-ast
```