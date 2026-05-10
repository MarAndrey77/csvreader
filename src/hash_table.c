#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hash_table.h"
#include "columns.h"
#include "rows.h"

unsigned int hash(const char* key, int table_size) {
    unsigned int h = 0;
    const unsigned char* p = (const unsigned char*)key;
    while (*p) {
        h = h * 31 + *p++;
    }
    return h % table_size;
}

HashTable* create_hash_table(int size) {
    HashTable* table = malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->entries = calloc(size, sizeof(HashEntry));
    for (int i = 0; i < size; i++) {
        table->entries[i].state = EMPTY;
        table->entries[i].key = NULL;
        table->entries[i].cell.formula = NULL;
    }
    
    return table;
}

int find_index(HashTable* table, const char* key) {
    int idx = hash(key, table->size);
    int start = idx;
    
    while (table->entries[idx].state != EMPTY) {
        if (table->entries[idx].state == OCCUPIED && 
            strcmp(table->entries[idx].key, key) == 0) {
            return idx;
        }
        idx = (idx + 1) % table->size;
        if (idx == start) break;
    }
    
    return -1;
}

void hash_table_insert(HashTable* table, const char* key, int value, int is_formula, const char* formula) {
    int idx = find_index(table, key);
    
    if (idx != -1) {
        table->entries[idx].cell.value = value;
        table->entries[idx].cell.is_formula = is_formula;
        table->entries[idx].cell.is_calculated = 0;
        if (table->entries[idx].cell.formula) {
            free(table->entries[idx].cell.formula);
        }
        table->entries[idx].cell.formula = formula ? strdup(formula) : NULL;
        return;
    }
    
    idx = hash(key, table->size);
    while (table->entries[idx].state == OCCUPIED) {
        idx = (idx + 1) % table->size;
    }

    table->entries[idx].key = strdup(key);
    table->entries[idx].cell.value = value;
    table->entries[idx].cell.is_formula = is_formula;
    table->entries[idx].cell.is_calculated = 0;
    table->entries[idx].cell.formula = formula ? strdup(formula) : NULL;
    table->entries[idx].state = OCCUPIED;
    table->count++;
}

Cell* hash_table_get(HashTable* table, const char* key) {
    int idx = find_index(table, key);
    if (idx != -1) {
        return &table->entries[idx].cell;
    }
    return NULL;
}

void hash_table_set_value(HashTable* table, const char* key, int value) {
    int idx = find_index(table, key);
    if (idx != -1) {
        table->entries[idx].cell.value = value;
        table->entries[idx].cell.is_calculated = 2;
    }
}

void free_hash_table(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        if (table->entries[i].state == OCCUPIED) {
            free(table->entries[i].key);
            if (table->entries[i].cell.formula) {
                free(table->entries[i].cell.formula);
            }
        }
    }
    free(table->entries);
    free(table);
}

int calculate_hash_table_size(Columns* columns, Rows* rows) {
    int size = columns->count * rows->count * 2;
    return size < 100 ? 100 : size;
}

char* make_cell_key(const char* column_name, int row_num) {
    int len = snprintf(NULL, 0, "%s%d", column_name, row_num);

    char* key = malloc(len + 1);
    if (key == NULL) {
        printf("Ошибка: не удалось выделить память под ключ ячейки\n");
        exit(1);
    }

    snprintf(key, len + 1, "%s%d", column_name, row_num);

    return key;
}

int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

void trim_spaces(char* str);

int parse_number_or_cell(HashTable* table, char* token) {
    trim_spaces(token);

    if (strlen(token) == 0) {
        printf("Ошибка: пустой операнд в формуле\n");
        exit(1);
    }

    char* endptr;
    long number = strtol(token, &endptr, 10);

    if (*endptr == '\0') {
        return (int)number;
    }

    Cell* cell = hash_table_get(table, token);

    if (cell == NULL) {
        printf("Ошибка: ссылка на несуществующую ячейку '%s'\n", token);
        exit(1);
    }

    return calculate_cell(table, token);
}

int evaluate_formula(HashTable* table, const char* formula) {
    char left[256];
    char right[256];

    int op_pos = -1;
    char op = '\0';
    
    int start_idx = 0;

    if (formula[0] == '-') {
        start_idx = 1;
    }
    for (int i = start_idx; formula[i] != '\0'; i++) {
        if (is_operator(formula[i])) {
            op_pos = i;
            op = formula[i];
            break;
        }
    }

    if (op_pos == -1) {
        char token[256];
        if (strlen(formula) >= sizeof(token)) {
            printf("Ошибка: слишком длинный операнд в формуле '%s'\n", formula);
            exit(1);
        }
        strcpy(token, formula);
        return parse_number_or_cell(table, token);
    }

    if (op_pos == 0 || formula[op_pos + 1] == '\0') {
        printf("Ошибка: некорректная формула '%s'\n", formula);
        exit(1);
    }

    if ((size_t)op_pos >= sizeof(left)) {
        printf("Ошибка: слишком длинная левая часть формулы '%s'\n", formula);
        exit(1);
    }
    strncpy(left, formula, op_pos);
    left[op_pos] = '\0';

    if (strlen(formula + op_pos + 1) >= sizeof(right)) {
        printf("Ошибка: слишком длинная правая часть формулы '%s'\n", formula);
        exit(1);
    }
    strcpy(right, formula + op_pos + 1);

    trim_spaces(left);
    trim_spaces(right);

    int left_value = parse_number_or_cell(table, left);
    int right_value = parse_number_or_cell(table, right);

    switch (op) {
        case '+': return left_value + right_value;
        case '-': return left_value - right_value;
        case '*': return left_value * right_value;
        case '/':
            if (right_value == 0) {
                printf("Ошибка: деление на ноль в формуле '%s'\n", formula);
                exit(1);
            }
            return left_value / right_value;
        default:
            printf("Ошибка: неизвестный оператор '%c'\n", op);
            exit(1);
    }
}

int calculate_cell(HashTable* table, const char* key) {
    Cell* cell = hash_table_get(table, key);

    if (cell == NULL) {
        printf("Ошибка: ячейка '%s' не найдена\n", key);
        exit(1);
    }

    if (!cell->is_formula) {
        return cell->value;
    }

    if (cell->is_calculated == 2) {
        return cell->value;
    }

    if (cell->is_calculated == 1) {
        printf("Ошибка: обнаружена циклическая зависимость с ячейкой '%s'\n", key);
        exit(1);
    }

    cell->is_calculated = 1;

    int result = evaluate_formula(table, cell->formula);

    cell->value = result;
    cell->is_calculated = 2;

    return result;
}

void calculate_all_formulas(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        if (table->entries[i].state == OCCUPIED) {
            calculate_cell(table, table->entries[i].key);
        }
    }
}