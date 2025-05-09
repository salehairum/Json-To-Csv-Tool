#include "schema.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* strclone(const char* s) {
    char* copy = (char*)malloc(strlen(s) + 1);
    strcpy(copy, s);
    return copy;
}

Table* create_table(const char* name) {
    Table* table = (Table*)malloc(sizeof(Table));
    table->name = strclone(name);
    table->columns = NULL;
    table->rows = NULL;
    table->next = NULL;
    return table;
}

void add_column(Table* table, const char* column) {
    Field* col = table->columns;
    while (col) {
        if (strcmp(col->name, column) == 0)
            return; // already exists
        col = col->next;
    }
    Field* new_col = (Field*)malloc(sizeof(Field));
    new_col->name = strclone(column);
    new_col->value = NULL;
    new_col->next = table->columns;
    table->columns = new_col;
}

Row* add_row(Table* table) {
    static int next_id = 1;
    Row* row = (Row*)malloc(sizeof(Row));
    row->id = next_id++;
    row->fields = NULL;
    row->next = table->rows;
    table->rows = row;
    return row;
}

int insert_row(Table* table, Field* fields, bool use_auto_id) {
    Row* row = malloc(sizeof(Row));
    row->id = use_auto_id ? table->next_id++ : -1;
    row->fields = fields;
    row->next = table->rows;
    table->rows = row;
    return row->id;
}


void add_field(Row* row, const char* key, const char* value) {
    Field* f = (Field*)malloc(sizeof(Field));
    f->name = strclone(key);
    f->value = strclone(value);
    f->next = row->fields;
    row->fields = f;
}

void print_table_schema(const Table* table) {
    printf("Table: %s\nColumns:", table->name);
    for (Field* col = table->columns; col; col = col->next)
        printf(" %s", col->name);
    printf("\n");
}

void print_table_data(const Table* table) {
    printf("Rows:\n");
    for (Row* row = table->rows; row; row = row->next) {
        printf("id=%d", row->id);
        for (Field* f = row->fields; f; f = f->next)
            printf(" %s=%s", f->name, f->value);
        printf("\n");
    }
}

void free_table(Table* table) {
    while (table->rows) {
        Row* r = table->rows;
        table->rows = r->next;
        while (r->fields) {
            Field* f = r->fields;
            r->fields = f->next;
            free(f->name);
            free(f->value);
            free(f);
        }
        free(r);
    }
    while (table->columns) {
        Field* f = table->columns;
        table->columns = f->next;
        free(f->name);
        free(f);
    }
    free(table->name);
    free(table);
}
