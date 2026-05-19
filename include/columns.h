#ifndef COLUMNS_H
#define COLUMNS_H

typedef struct {
    int count;
    int capacity;
    char** data;
} Columns;

void init_columns(Columns* columns);
void add_column(Columns* columns, char* new_column);
void validate_unique_columns(Columns* columns);
void free_columns(Columns* columns);

#endif