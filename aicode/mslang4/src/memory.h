#ifndef MS_MEMORY_H
#define MS_MEMORY_H

#include "common.h"

void* ms_reallocate(void* pointer, size_t oldSize, size_t newSize);

#define MS_ALLOCATE(type, count) \
    (type*)ms_reallocate(NULL, 0, sizeof(type) * (count))

#define MS_FREE(type, pointer, count) \
    ms_reallocate(pointer, sizeof(type) * (count), 0)

#define MS_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define MS_GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)ms_reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define MS_FREE_ARRAY(type, pointer, oldCount) \
    ms_reallocate(pointer, sizeof(type) * (oldCount), 0)

#endif
