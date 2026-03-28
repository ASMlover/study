#ifndef MSLANGC_STRING_H_
#define MSLANGC_STRING_H_

#include <stddef.h>
#include <stdint.h>

#include "ms/object.h"

typedef struct MsString {
  MsObject object;
  size_t length;
  uint32_t hash;
  char *bytes;
} MsString;

/* Copies the provided UTF-8 bytes into a new heap string object. */
MsString *ms_string_new(const char *bytes, size_t length);
MsString *ms_string_from_cstr(const char *cstring);
void ms_string_free(MsString *string);
int ms_string_equals(const MsString *left, const MsString *right);
uint32_t ms_string_hash_bytes(const char *bytes, size_t length);

#endif