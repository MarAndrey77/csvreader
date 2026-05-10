#ifndef ROWS_H
#define ROWS_H

typedef struct {
    int count;
    int capacity;
    int* data;
} Rows;

void init_rows(Rows* rows);
void add_row(Rows* rows, int new_row);
void validate_unique_rows(Rows* rows);
int compare_ints(const void* a, const void* b); 

#endif