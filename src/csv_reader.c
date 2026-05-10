#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csv_reader.h"
#include "hash_table.h"

#define MAX_CELL_CONTENT 256

void trim_spaces(char* str) {
    int start = 0;

    while (isspace((unsigned char)str[start])) {
        start++;
    }

    int end = strlen(str) - 1;

    while (end >= start && isspace((unsigned char)str[end])) {
        str[end] = '\0';
        end--;
    }

    if (start > 0) {
        memmove(str, str + start, strlen(str + start) + 1);
    }
}

int read_field(FILE* file, char* buffer, int buffer_size) {
    int i = 0;
    int c;
    
    while ((c = fgetc(file)) != EOF) {
        if (c == ',' || c == '\n') {
            buffer[i] = '\0';
            return c;
        } 
        if (i >= buffer_size - 1) {
            printf("Ошибка: длина содержимого ячейки превышает лимит в %d символов\n", buffer_size - 1);
            return -2;
        }
        buffer[i++] = c;
    }
    
    buffer[i] = '\0';
    return EOF;
}

int validate_column_name(const char* name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (isspace((unsigned char)name[i])) {
            fprintf(stderr, "Ошибка: название столбца '%s' содержит пробелы внутри\n", name);
            return 0;
        }
    }
    return 1;
}

void read_columns(FILE* file, Columns* columns) {
    char field[MAX_CELL_CONTENT];
    int next_symb;
    
    next_symb = read_field(file, field, sizeof(field));
    
    while (next_symb != '\n' && next_symb != EOF) {
        next_symb = read_field(file, field, sizeof(field));
        if (next_symb == -1) exit(1);
        
        trim_spaces(field);
        
        if (strlen(field) > 0) {
            if (!validate_column_name(field)) {
                exit(1);
            }
            add_column(columns, field);
        }
    }
}

int skip_to_next_line(FILE* file, int current_delimiter) {
    int c;

    if (current_delimiter == '\n' || current_delimiter == EOF) {
        return current_delimiter;
    }

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            return '\n';
        }
    }

    return EOF;
}

void read_rows(FILE* file, Rows* rows) {
    char field[32];
    int delimiter;
    int row_num;
    int chars_consumed;

    while (1) {
        delimiter = read_field(file, field, sizeof(field));

        if (delimiter == EOF && strlen(field) == 0) {
            break;
        }
        trim_spaces(field);

        if (strlen(field) == 0 && delimiter == '\n') {
            continue;
        }
        
        if (strlen(field) > 7) {
            printf("Ошибка: номер строки '%s' превышает 7 цифр\n", field);
            exit(1);
        }
        
        for (int i = 0; field[i]; i++) {
            if (!isdigit((unsigned char)field[i])) {
                printf("Ошибка: номер строки не является целым положительным числом '%s'\n", field);
                exit(1);
            }
        }
        
        if (sscanf(field, "%d%n", &row_num, &chars_consumed) != 1) {
            printf("Ошибка: не удалось преобразовать '%s' в число\n", field);
            exit(1);
        }

        add_row(rows, row_num);

        if (delimiter == ',') {
            delimiter = skip_to_next_line(file, delimiter);
        }

        if (delimiter == EOF) {
            break;
        }
    }
}

void skip_header_line(FILE* file) {
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            return;
        }
    }
}

int parse_cell_value(const char* field, int* value) {
    char* endptr;

    long result = strtol(field, &endptr, 10);

    if (*endptr != '\0') {
        return 0;
    }

    *value = (int)result;
    return 1;
}

HashTable* fill_hash_table(FILE* file, Columns* columns, Rows* rows) {
    int table_size = calculate_hash_table_size(columns, rows);
    HashTable* table = create_hash_table(table_size);
    
    rewind(file);
    skip_header_line(file);
    
    char field[MAX_CELL_CONTENT];
    int delimiter;
    
    for (int row_idx = 0; row_idx < rows->count; row_idx++) {
        int current_row_num = rows->data[row_idx];
        
        delimiter = read_field(file, field, sizeof(field));
        
        if (delimiter == EOF && strlen(field) == 0) {
            printf("Ошибка: данных меньше, чем ожидалось\n");
            exit(1);
        }
        
        for (int col_idx = 0; col_idx < columns->count; col_idx++) {
            delimiter = read_field(file, field, sizeof(field));
            if (delimiter == -2) {
                printf("Ошибка: переполнение буфера\n");
                exit(1);
            }
            
            trim_spaces(field);
            
            char* key = make_cell_key(columns->data[col_idx], current_row_num);
            
            if (field[0] == '=') {
                char* formula = field + 1;
                trim_spaces(formula);
                hash_table_insert(table, key, 0, 1, formula);
            } 
            else if (strlen(field) == 0) {
                printf("Ошибка: пустая ячейка %s\n", key);
                free(key);
                exit(1);
            } 
            else {
                int value;
                if (!parse_cell_value(field, &value)) {
                    printf("Ошибка: некорректное значение ячейки %s: '%s'\n", key, field);
                    free(key);
                    exit(1);
                }
                hash_table_insert(table, key, value, 0, NULL);
            }
            
            free(key);
            
            if (col_idx < columns->count - 1) {
                if (delimiter != ',') {
                    printf("Ошибка: в строке %d меньше ячеек, чем столбцов\n", current_row_num);
                    exit(1);
                }
            } 
            else {
                if (delimiter == ',') {
                    printf("Ошибка: в строке %d больше ячеек, чем столбцов\n", current_row_num);
                    exit(1);
                }
            }
        }
        
        if (delimiter == EOF) {
            if (row_idx < rows->count - 1) {
                printf("Ошибка: файл закончился раньше, чем ожидалось (%d строк из %d)\n", 
                       row_idx + 1, rows->count);
                exit(1);
            }
            break;
        }
    }
    
    return table;
}

void print_table(Columns* columns, Rows* rows, HashTable* cells) {
    qsort(rows->data, rows->count, sizeof(int), compare_ints);

    for (int i = 0; i < columns->count; i++) {
        printf(",%s", columns->data[i]);
    }
    printf("\n");
    
    for (int i = 0; i < rows->count; i++) {
        int row_num = rows->data[i];
        printf("%d", row_num);
        
        for (int j = 0; j < columns->count; j++) {
            char key[64];
            sprintf(key, "%s%d", columns->data[j], row_num);
            Cell* cell = hash_table_get(cells, key);
            printf(",%d", cell->value);
        }
        printf("\n");
    }
}