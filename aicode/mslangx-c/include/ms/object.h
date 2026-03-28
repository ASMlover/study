#ifndef MSLANGC_OBJECT_H_
#define MSLANGC_OBJECT_H_

typedef enum MsObjectType {
  MS_OBJ_STRING,
  MS_OBJ_FUNCTION,
  MS_OBJ_CLOSURE,
  MS_OBJ_UPVALUE,
  MS_OBJ_CLASS,
  MS_OBJ_INSTANCE,
  MS_OBJ_BOUND_METHOD,
  MS_OBJ_NATIVE_FN,
  MS_OBJ_LIST,
  MS_OBJ_TUPLE,
  MS_OBJ_MAP,
  MS_OBJ_MODULE
} MsObjectType;

typedef struct MsObject {
  MsObjectType type;
  unsigned char marked;
  struct MsObject *next;
} MsObject;

void ms_object_init(MsObject *object, MsObjectType type);
const char *ms_object_type_name(MsObjectType type);

#endif