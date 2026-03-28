#include "ms/buffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool ms_buffer_can_grow(size_t min_capacity) {
  return min_capacity <= (SIZE_MAX / 2);
}

void ms_buffer_init(MsBuffer *buffer) {
  buffer->data = NULL;
  buffer->length = 0;
  buffer->capacity = 0;
}

bool ms_buffer_reserve(MsBuffer *buffer, size_t min_capacity) {
  size_t new_capacity;
  unsigned char *data;

  if (!buffer) {
    return false;
  }
  if (min_capacity <= buffer->capacity) {
    return true;
  }
  if (!ms_buffer_can_grow(min_capacity)) {
    return false;
  }

  new_capacity = buffer->capacity ? buffer->capacity : 16;
  while (new_capacity < min_capacity) {
    if (new_capacity > (SIZE_MAX / 2)) {
      new_capacity = min_capacity;
      break;
    }
    new_capacity *= 2;
  }

  data = realloc(buffer->data, new_capacity);
  if (!data) {
    return false;
  }

  buffer->data = data;
  buffer->capacity = new_capacity;
  return true;
}

bool ms_buffer_append(MsBuffer *buffer, const void *data, size_t size) {
  size_t new_length;

  if (!buffer) {
    return false;
  }
  if (size == 0) {
    return true;
  }
  if (size > SIZE_MAX - buffer->length) {
    return false;
  }

  new_length = buffer->length + size;
  if (!ms_buffer_reserve(buffer, new_length)) {
    return false;
  }

  memcpy(buffer->data + buffer->length, data, size);
  buffer->length = new_length;
  return true;
}

bool ms_buffer_write(MsBuffer *buffer, size_t offset, const void *data,
                     size_t size) {
  if (!buffer) {
    return false;
  }
  if (size == 0) {
    return offset <= buffer->length;
  }
  if (offset > buffer->length || size > buffer->length - offset) {
    return false;
  }

  memcpy(buffer->data + offset, data, size);
  return true;
}

void ms_buffer_clear(MsBuffer *buffer) {
  if (!buffer) {
    return;
  }

  buffer->length = 0;
}

void ms_buffer_destroy(MsBuffer *buffer) {
  if (!buffer) {
    return;
  }

  free(buffer->data);
  buffer->data = NULL;
  buffer->length = 0;
  buffer->capacity = 0;
}