#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "columns.h"
#include "rows.h"

typedef struct {
    int value;
    int is_formula;
    int is_calculated;
    char* formula;
} Cell;

typedef enum {
    EMPTY,
    OCCUPIED
} EntryState;

typedef struct {
    char* key;
    Cell cell;
    EntryState state;
} HashEntry;

typedef struct {
    HashEntry* entries;
    int size;
    int count;
} HashTable;

int calculate_cell(HashTable* table, const char* key);
HashTable* create_hash_table(int size);
void hash_table_insert(HashTable* table, const char* key, int value, int is_formula, const char* formula);
Cell* hash_table_get(HashTable* table, const char* key);
void hash_table_set_value(HashTable* table, const char* key, int value);
void free_hash_table(HashTable* table);
char* make_cell_key(const char* column_name, int row_num);
int calculate_hash_table_size(Columns* columns, Rows* rows);
void calculate_all_formulas(HashTable* table);

#endif