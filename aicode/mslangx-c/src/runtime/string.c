#include "ms/string.h"

#include <stdlib.h>
#include <string.h>

uint32_t ms_string_hash_bytes(const char *bytes, size_t length) {
  uint32_t hash = 2166136261u;
  size_t i;

  for (i = 0; i < length; ++i) {
    hash ^= (unsigned char) bytes[i];
    hash *= 16777619u;
  }

  return hash;
}

MsString *ms_string_new(const char *bytes, size_t length) {
  MsString *string;
  char *storage;

  if (bytes == NULL && length != 0) {
    return NULL;
  }

  string = (MsString *) malloc(sizeof(*string));
  if (string == NULL) {
    return NULL;
  }

  storage = (char *) malloc(length + 1);
  if (storage == NULL) {
    free(string);
    return NULL;
  }

  if (length != 0) {
    memcpy(storage, bytes, length);
  }
  storage[length] = '\0';

  ms_object_init(&string->object, MS_OBJ_STRING);
  string->length = length;
  string->hash = ms_string_hash_bytes(storage, length);
  string->bytes = storage;
  return string;
}

MsString *ms_string_from_cstr(const char *cstring) {
  if (cstring == NULL) {
    return NULL;
  }

  return ms_string_new(cstring, strlen(cstring));
}

void ms_string_free(MsString *string) {
  if (string == NULL) {
    return;
  }

  free(string->bytes);
  free(string);
}

int ms_string_equals(const MsString *left, const MsString *right) {
  if (left == right) {
    return 1;
  }

  if (left == NULL || right == NULL) {
    return 0;
  }

  if (left->length != right->length || left->hash != right->hash) {
    return 0;
  }

  return memcmp(left->bytes, right->bytes, left->length) == 0;
}