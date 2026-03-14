//
// Created by berke on 3/14/2026.
//

#include "../../Headers/Objects/Object.h"

#include <stdlib.h>

void objects_create_objects_list(ObjectsList* list, int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc((size_t)list->size * sizeof(*list->items));
}

void objects_push_objects_list(ObjectsList* list, Object* value) {
    if (++list->count > list->size) {
        list->size += list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
    list->items[list->count - 1] = *value;
}

void objects_pop_objects_list(ObjectsList* list) {
    if (list->count <= 0) {
        return;
    }

    list->count--;

    if (list->size - list->count >= list->chunkSize && list->size > list->chunkSize) {
        list->size -= list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
}

void objects_free_objects_list(ObjectsList* list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->size = 0;
    list->chunkSize = 0;
}