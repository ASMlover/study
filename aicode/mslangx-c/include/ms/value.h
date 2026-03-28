#ifndef MSLANGC_VALUE_H_
#define MSLANGC_VALUE_H_

#include <stddef.h>

#include "ms/object.h"

typedef enum MsValueType {
  MS_VAL_NIL,
  MS_VAL_BOOL,
  MS_VAL_NUMBER,
  MS_VAL_OBJECT
} MsValueType;

typedef struct MsString MsString;

typedef struct MsValue {
  MsValueType type;
  union {
    int boolean;
    double number;
    MsObject *object;
  } as;
} MsValue;

MsValue ms_value_nil(void);
MsValue ms_value_bool(int boolean);
MsValue ms_value_number(double number);
MsValue ms_value_object(MsObject *object);

int ms_value_is_nil(MsValue value);
int ms_value_is_bool(MsValue value);
int ms_value_is_number(MsValue value);
int ms_value_is_object(MsValue value);
int ms_value_is_string(MsValue value);

int ms_value_get_bool(MsValue value, int *out_boolean);
int ms_value_get_number(MsValue value, double *out_number);
int ms_value_get_object(MsValue value, MsObject **out_object);
int ms_value_get_string(MsValue value, MsString **out_string);

int ms_value_is_falsey(MsValue value);
int ms_value_equals(MsValue left, MsValue right);
int ms_value_format(MsValue value, char *buffer, size_t buffer_size);

#endif