#include "value.hh"

namespace tadpole {

StringObject* BaseObject::as_string() {
  return nullptr;
}

const char* BaseObject::as_cstring() {
  return nullptr;
}

NativeObject* BaseObject::as_native() {
  return nullptr;
}

FunctionObject* BaseObject::as_function() {
  return nullptr;
}

UpvalueObject* BaseObject::as_upvalue() {
  return nullptr;
}

ClosureObject* BaseObject::as_closure() {
  return nullptr;
}

bool Value::is_truthy() const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric != 0;
  case ValueType::OBJECT: return as_.object->is_truthy();
  }
  return false;
}

str_t Value::stringify() const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return tadpole::as_string(as_.numeric);
  case ValueType::OBJECT: return as_.object->stringify();
  }
  return "";
}

}