#include "ast.h"
#include "schema.h"
#include "semantics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global list of tables
static Table* all_tables = NULL;

// Utility: Find or create a table by name
Table* get_or_create_table(const char* name) {
    Table* t = all_tables;
    while (t) {
        if (strcmp(t->name, name) == 0) return t;
        t = t->next;
    }
    // Not found, create new
    Table* new_table = create_table(name);
    new_table->next = all_tables;
    all_tables = new_table;
    return new_table;
}

// Turn object key set into table name (simplified)
const char* guess_table_name(ASTNode* pair) {
    // Use the key as table name if available
    if (pair && pair->key) {
        return pair->key;
    }
    return "data";
}


// Recursive function to process AST
void process_ast(ASTNode* node, const char* parent_table, int parent_id) {
    if (!node) return;

    switch (node->type) {
        case AST_OBJECT: {
            const char* table_name = guess_table_name(node->children);
            Table* table = get_or_create_table(table_name);
            Row* row = add_row(table);

            if (parent_table) {
                char fk_col[256];
                snprintf(fk_col, sizeof(fk_col), "%s_id", parent_table);
                add_column(table, fk_col);
                char id_str[20]; sprintf(id_str, "%d", parent_id);
                add_field(row, fk_col, id_str);
            }

            ASTNode* pair = node->children;
            while (pair) {
                if (pair->type == AST_PAIR) {
                    add_column(table, pair->key);
                    if (pair->value) {
                        if (pair->value->type == AST_STRING || pair->value->type == AST_NUMBER) {
                            add_field(row, pair->key, pair->value->strval);
                        } else if (pair->value->type == AST_OBJECT || pair->value->type == AST_ARRAY) {
                            // Nested structure → recurse
                            process_ast(pair->value, table->name, row->id);
                        } else if (pair->value->type == AST_TRUE) {
                            add_field(row, pair->key, "true");
                        } else if (pair->value->type == AST_FALSE) {
                            add_field(row, pair->key, "false");
                        } else if (pair->value->type == AST_NULL) {
                            add_field(row, pair->key, "");
                        }
                    }
                }
                pair = pair->next;
            }
            break;
        }
        case AST_ARRAY: {
            int index = 0;
            ASTNode* child = node->children;
            while (child) {
                process_ast(child, parent_table, parent_id);
                child = child->next;
                index++;
            }
            break;
        }
        default:
            // Primitive values only appear inside objects, so do nothing here
            break;
    }
}

void process_ast_with_key(const char* key, ASTNode* node, const char* parent_table, int parent_id) {
    if (!node) return;

    if (node->type == AST_OBJECT) {
        Table* table = get_or_create_table(key);
        Row* row = add_row(table);  // this assigns a real ID

        // Add foreign key to parent
        if (parent_table) {
            char fk_col[256];
            snprintf(fk_col, sizeof(fk_col), "%s_id", parent_table);
            add_column(table, fk_col);

            char id_str[20];
            sprintf(id_str, "%d", parent_id);
            add_field(row, fk_col, id_str);
        }

        ASTNode* pair = node->children;
        while (pair) {
            if (pair->type == AST_PAIR) {
                add_column(table, pair->key);
                if (pair->value) {
                    if (pair->value->type == AST_STRING || pair->value->type == AST_NUMBER) {
                        add_field(row, pair->key, pair->value->strval);
                    } else if (pair->value->type == AST_OBJECT || pair->value->type == AST_ARRAY) {
                        // Recurse into nested object or array
                        process_ast_with_key(pair->key, pair->value, table->name, row->id);
                    } else if (pair->value->type == AST_TRUE) {
                        add_field(row, pair->key, "true");
                    } else if (pair->value->type == AST_FALSE) {
                        add_field(row, pair->key, "false");
                    } else if (pair->value->type == AST_NULL) {
                        add_field(row, pair->key, "");
                    }
                }
            }
            pair = pair->next;
        }
    }

    else if (node->type == AST_ARRAY) {
        int index = 0;
        ASTNode* child = node->children;

        while (child) {
            if (child->type == AST_STRING || child->type == AST_NUMBER) {
                Table* table = get_or_create_table(key);

                // Manually construct a Row struct with id = -1 to skip it in CSV
                Row* row = malloc(sizeof(Row));
                row->id = -1;
                row->fields = NULL;
                row->next = table->rows;
                table->rows = row;

                // Add foreign key
                if (parent_table) {
                    char fk_col[256];
                    snprintf(fk_col, sizeof(fk_col), "%s_id", parent_table);
                    add_column(table, fk_col);

                    char id_str[20];
                    sprintf(id_str, "%d", parent_id);
                    add_field(row, fk_col, id_str);
                }

                // Add index
                add_column(table, "index");
                char index_str[20];
                sprintf(index_str, "%d", index);
                add_field(row, "index", index_str);

                // Add value
                add_column(table, "value");
                add_field(row, "value", child->strval);

                index++;
            } else {
                // Nested object/array
                process_ast_with_key(key, child, parent_table, parent_id);
            }
            child = child->next;
        }
    }

    else {
        // Fallback: basic node
        process_ast(node, parent_table, parent_id);
    }
}


// Expose final tables
Table* get_all_tables() {
    return all_tables;
}

bool is_nested_table(char* name) {
    Table* t = get_all_tables();
    while (t) {
        if (strcmp(t->name, name) == 0)
            return true;
        t = t->next;
    }
    return false;
}


void print_all_tables(Table* t) {
    while (t) {
        printf("\nTable: %s\n", t->name);
        printf("Columns: \n");
        Field* col = t->columns;
        printf("id        |");
        while (col) {
            printf("%-10s |", col->name);
            col = col->next;
        }
        printf("\n");

        Row* row = t->rows;
        while (row) {
            printf("id=%d |", row->id);
            Field* f = row->fields;
            Field* col = t->columns;
            while (col) {
                Field* temp = f;
                // Find matching field by name
                while (temp && strcmp(temp->name, col->name) != 0) {
                    temp = temp->next;
                }
                // Inside print_all_tables(), when printing values:
                if (temp)
                    printf(" %-10s |", temp->value);
                else if (is_nested_table(col->name))
                    printf(" %-10s |", col->name);  // ← print the nested table's name
                else
                    printf(" %-10s |", "NULL");

                col = col->next;
            }
            printf("\n");
            row = row->next;
        }

        t = t->next;
    }
}

void write_quoted(FILE* fp, const char* value) {
    if (!value || strcmp(value, "NULL") == 0) {
        // Write empty cell for NULLs
        return;
    }
    fprintf(fp, "\"");
    for (const char* p = value; *p; ++p) {
        if (*p == '"') fprintf(fp, "\"\"");  // Escape double quotes
        else fputc(*p, fp);
    }
    fprintf(fp, "\"");
}
void write_table_to_csv(Table* table, const char* output_dir) {
    char filename[512];
    if (output_dir && strlen(output_dir) > 0) {
        snprintf(filename, sizeof(filename), "%s/%s.csv", output_dir, table->name);
    } else {
        snprintf(filename, sizeof(filename), "%s.csv", table->name);
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open CSV file");
        return;
    }

    // --- Write Header ---
    bool first = true;

    if (table->rows && table->rows->id != -1) {
        fprintf(fp, "id");
        first = false;
    }

    Field* header_field = table->rows ? table->rows->fields : NULL;
    while (header_field) {
        if (!first) fprintf(fp, ",");
        fprintf(fp, "%s", header_field->name);
        header_field = header_field->next;
        first = false;
    }
    fprintf(fp, "\n");

    // --- Write Rows ---
    for (Row* row = table->rows; row != NULL; row = row->next) {
        first = true;
        if (row->id != -1) {
            fprintf(fp, "%d", row->id);
            first = false;
        }

        for (Field* field = row->fields; field != NULL; field = field->next) {
            if (!first) fprintf(fp, ",");
            fprintf(fp, "\"%s\"", field->value);  // CSV-safe values
            first = false;
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}