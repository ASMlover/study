#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void* ms_reallocate(void* pointer, size_t oldSize, size_t newSize) {
    (void)oldSize;
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) {
        fprintf(stderr, "Maple: out of memory\n");
        exit(1);
    }
    return result;
}
