#include <stdio.h>
#include <stdlib.h>
#include "columns.h"
#include "rows.h"
#include "hash_table.h"
#include "csv_reader.h"

int main(int argc, char* argv[]){
    char* path = argv[1];
    
    Columns* columns = malloc(sizeof(Columns));
    Rows* rows = malloc(sizeof(Rows));
    
    init_columns(columns);
    init_rows(rows);
    
    FILE* file = fopen(path, "r");
    read_columns(file, columns);
    read_rows(file, rows);
    
    validate_unique_columns(columns);
    validate_unique_rows(rows);
    
    HashTable* table = fill_hash_table(file, columns, rows);
    calculate_all_formulas(table);
    print_table(columns, rows, table);
 
    free_hash_table(table);   
    free_columns(columns);   
    free_rows(rows);        
    fclose(file);              
    
    return 0;
}