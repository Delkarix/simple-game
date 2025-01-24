#include <stdlib.h>
#include "utils.h"
#include "stdint.h"

void insertIntoList(List** list, uint16_t item) {
    if ((*list)->length >= (*list)->capacity) {
        *list = realloc(*list, (*list)->capacity * 2); // Doing bitshit because 1 << 0 = 1 instead of 0
        (*list)->capacity *= 2;
    }

    (*list)->elements[(*list)->length++] = item;
}

void removeAt(List* list, uint16_t i) {
    // NOTE: NO SAFETY BOUNDS
    list->elements[i] = list->elements[list->length--];
}