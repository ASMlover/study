#include "ms/object.h"

#include <stddef.h>

void ms_object_init(MsObject *object, MsObjectType type) {
  if (object == NULL) {
    return;
  }

  object->type = type;
  object->marked = 0;
  object->next = NULL;
}

const char *ms_object_type_name(MsObjectType type) {
  switch (type) {
    case MS_OBJ_STRING:
      return "string";
    case MS_OBJ_FUNCTION:
      return "function";
    case MS_OBJ_CLOSURE:
      return "closure";
    case MS_OBJ_UPVALUE:
      return "upvalue";
    case MS_OBJ_CLASS:
      return "class";
    case MS_OBJ_INSTANCE:
      return "instance";
    case MS_OBJ_BOUND_METHOD:
      return "bound_method";
    case MS_OBJ_NATIVE_FN:
      return "native_function";
    case MS_OBJ_LIST:
      return "list";
    case MS_OBJ_TUPLE:
      return "tuple";
    case MS_OBJ_MAP:
      return "map";
    case MS_OBJ_MODULE:
      return "module";
  }

  return "object";
}