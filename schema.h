#include <stdbool.h>

#ifndef SCHEMA_H
#define SCHEMA_H

typedef struct Field {
    char* name;
    char* value;
    struct Field* next;
} Field;

typedef struct Row {
    int id;              // primary key
    Field* fields;       // linked list of field name=value pairs
    struct Row* next;
} Row;

// typedef struct Table {
//     char* name;
//     Field* columns;      // column names (no values)
//     Row* rows;           // linked list of rows
//     struct Table* next;
// } Table;

typedef struct Table {
    char* name;
    Field* columns;
    Row* rows;
    int next_id;             // ADD THIS LINE
    struct Table* next;
} Table;


// API
Table* create_table(const char* name);
void add_column(Table* table, const char* column);
Row* add_row(Table* table);
void add_field(Row* row, const char* key, const char* value);
int insert_row(Table* table, Field* fields, bool use_auto_id);

void print_table_schema(const Table* table);
void print_table_data(const Table* table);
void free_table(Table* table);

#endif
