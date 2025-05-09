#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"
#include "schema.h"

void process_ast(ASTNode* node, const char* parent_table, int parent_id);
void process_ast_with_key(const char* key, ASTNode* node, const char* parent_table, int parent_id);
Table* get_all_tables(void);
void print_all_tables();
void write_table_to_csv(Table* table, const char* output_dir);

#endif
