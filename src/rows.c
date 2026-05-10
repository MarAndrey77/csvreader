#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rows.h"

void init_rows(Rows* rows) {
    rows->count = 0;
    rows->capacity = 100;
    rows->data = malloc(rows->capacity * sizeof(int));
}

void add_row(Rows* rows, int new_row) {
    if (rows->count >= rows->capacity) {
        rows->capacity *= 2;
        rows->data = realloc(rows->data, rows->capacity * sizeof(int));
    }
    rows->data[rows->count++] = new_row;
}

int compare_ints(const void* a, const void* b) {
    int x = *(const int*)a;
    int y = *(const int*)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

void validate_unique_rows(Rows* rows) {
    int* copy = malloc(rows->count * sizeof(int));

    if (copy == NULL) {
        printf("Ошибка: не удалось выделить память\n");
        exit(1);
    }

    memcpy(copy, rows->data, rows->count * sizeof(int));

    qsort(copy, rows->count, sizeof(int), compare_ints);

    for (int i = 1; i < rows->count; i++) {
        if (copy[i] == copy[i - 1]) {
            printf("Ошибка: таблица невалидна — номер строки %d повторяется\n", copy[i]);
            free(copy);
            exit(1);
        }
    }

    free(copy);
}