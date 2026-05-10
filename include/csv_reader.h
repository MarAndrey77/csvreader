#ifndef CSV_READER_H
#define CSV_READER_H

#include <stdio.h>
#include "columns.h"
#include "rows.h"
#include "hash_table.h"

int read_field(FILE* file, char* buffer, int buffer_size);
void read_columns(FILE* file, Columns* columns);
void read_rows(FILE* file, Rows* rows);
HashTable* fill_hash_table(FILE* file, Columns* columns, Rows* rows);
void print_table(Columns* columns, Rows* rows, HashTable* cells);
void skip_header_line(FILE* file);
void trim_spaces(char* str);

#endif