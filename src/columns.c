#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "columns.h"

void init_columns(Columns* columns) {
    columns->count = 0;
    columns->capacity = 10;
    columns->data = malloc(columns->capacity * sizeof(char*));
}

void add_column(Columns* columns, char* new_column) {
    if (columns->count >= columns->capacity) {
        columns->capacity *= 2;
        columns->data = realloc(columns->data, columns->capacity * sizeof(char*));
    }
    columns->data[columns->count++] = strdup(new_column);
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void validate_unique_columns(Columns* columns) {
    char** copy = malloc(columns->count * sizeof(char*));

    if (copy == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    memcpy(copy, columns->data, columns->count * sizeof(char*));

    qsort(copy, columns->count, sizeof(char*), compare_strings);

    for (int i = 1; i < columns->count; i++) {
        if (strcmp(copy[i], copy[i - 1]) == 0) {
            printf("Ошибка: таблица невалидна — название столбца '%s' повторяется\n", copy[i]);
            free(copy);
            exit(1);
        }
    }

    free(copy);
}