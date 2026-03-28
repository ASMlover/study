#ifndef MS_BUFFER_H_
#define MS_BUFFER_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct MsBuffer {
  unsigned char *data;
  size_t length;
  size_t capacity;
} MsBuffer;

void ms_buffer_init(MsBuffer *buffer);
bool ms_buffer_reserve(MsBuffer *buffer, size_t min_capacity);
bool ms_buffer_append(MsBuffer *buffer, const void *data, size_t size);
bool ms_buffer_write(MsBuffer *buffer, size_t offset, const void *data,
                     size_t size);
void ms_buffer_clear(MsBuffer *buffer);
void ms_buffer_destroy(MsBuffer *buffer);

#endif