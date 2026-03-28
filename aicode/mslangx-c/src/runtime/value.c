#include "ms/value.h"

#include <stdio.h>
#include <string.h>

#include "ms/string.h"

MsValue ms_value_nil(void) {
  MsValue value;

  value.type = MS_VAL_NIL;
  value.as.number = 0.0;
  return value;
}

MsValue ms_value_bool(int boolean) {
  MsValue value;

  value.type = MS_VAL_BOOL;
  value.as.boolean = boolean != 0;
  return value;
}

MsValue ms_value_number(double number) {
  MsValue value;

  value.type = MS_VAL_NUMBER;
  value.as.number = number;
  return value;
}

MsValue ms_value_object(MsObject *object) {
  MsValue value;

  value.type = MS_VAL_OBJECT;
  value.as.object = object;
  return value;
}

int ms_value_is_nil(MsValue value) {
  return value.type == MS_VAL_NIL;
}

int ms_value_is_bool(MsValue value) {
  return value.type == MS_VAL_BOOL;
}

int ms_value_is_number(MsValue value) {
  return value.type == MS_VAL_NUMBER;
}

int ms_value_is_object(MsValue value) {
  return value.type == MS_VAL_OBJECT;
}

int ms_value_is_string(MsValue value) {
  return value.type == MS_VAL_OBJECT && value.as.object != NULL &&
         value.as.object->type == MS_OBJ_STRING;
}

int ms_value_get_bool(MsValue value, int *out_boolean) {
  if (!ms_value_is_bool(value) || out_boolean == NULL) {
    return 0;
  }

  *out_boolean = value.as.boolean;
  return 1;
}

int ms_value_get_number(MsValue value, double *out_number) {
  if (!ms_value_is_number(value) || out_number == NULL) {
    return 0;
  }

  *out_number = value.as.number;
  return 1;
}

int ms_value_get_object(MsValue value, MsObject **out_object) {
  if (!ms_value_is_object(value) || out_object == NULL) {
    return 0;
  }

  *out_object = value.as.object;
  return 1;
}

int ms_value_get_string(MsValue value, MsString **out_string) {
  if (!ms_value_is_string(value) || out_string == NULL) {
    return 0;
  }

  *out_string = (MsString *) value.as.object;
  return 1;
}

int ms_value_is_falsey(MsValue value) {
  MsString *string;

  if (value.type == MS_VAL_NIL) {
    return 1;
  }

  if (value.type == MS_VAL_BOOL) {
    return value.as.boolean == 0;
  }

  if (value.type == MS_VAL_NUMBER) {
    return value.as.number == 0.0;
  }

  if (ms_value_get_string(value, &string)) {
    return string->length == 0;
  }

  return 0;
}

int ms_value_equals(MsValue left, MsValue right) {
  MsString *left_string;
  MsString *right_string;

  if (left.type != right.type) {
    return 0;
  }

  switch (left.type) {
    case MS_VAL_NIL:
      return 1;
    case MS_VAL_BOOL:
      return left.as.boolean == right.as.boolean;
    case MS_VAL_NUMBER:
      return left.as.number == right.as.number;
    case MS_VAL_OBJECT:
      if (left.as.object == right.as.object) {
        return 1;
      }

      if (!ms_value_get_string(left, &left_string) ||
          !ms_value_get_string(right, &right_string)) {
        return 0;
      }

      return ms_string_equals(left_string, right_string);
  }

  return 0;
}

int ms_value_format(MsValue value, char *buffer, size_t buffer_size) {
  int written = -1;
  MsString *string;

  if (buffer == NULL || buffer_size == 0) {
    return 0;
  }

  switch (value.type) {
    case MS_VAL_NIL:
      written = snprintf(buffer, buffer_size, "nil");
      break;
    case MS_VAL_BOOL:
      written = snprintf(buffer, buffer_size,
                         value.as.boolean ? "true" : "false");
      break;
    case MS_VAL_NUMBER:
      written = snprintf(buffer, buffer_size, "%.17g", value.as.number);
      break;
    case MS_VAL_OBJECT:
      if (ms_value_get_string(value, &string)) {
        written = snprintf(buffer, buffer_size, "%s", string->bytes);
      } else if (value.as.object != NULL) {
        written = snprintf(buffer, buffer_size, "<%s>",
                           ms_object_type_name(value.as.object->type));
      } else {
        written = snprintf(buffer, buffer_size, "<null-object>");
      }
      break;
  }

  return written >= 0 && (size_t) written < buffer_size;
}