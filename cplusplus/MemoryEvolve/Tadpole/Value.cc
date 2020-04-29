#include <Tadpole/Value.hh>

namespace _mevo::tadpole {

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

}