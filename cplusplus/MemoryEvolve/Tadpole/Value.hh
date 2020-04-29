#pragma once

#include <functional>
#include <Core/MemoryEvolve.hh>

namespace _mevo::tadpole {

enum class ObjType {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
};

class VM;
class Chunk;

class StringObject;
class NativeObject;
class FunctionObject;
class UpvalueObject;
class ClosureObject;

class BaseObject : private UnCopyable {
  ObjType type_;
  bool marked_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool marked() const noexcept { return marked_; }
  inline void set_marked(bool marked = true) noexcept { marked_ = marked; }

  virtual str_t stringify() const { return "<object>"; }
  virtual bool is_truthy() const { return true; }
  virtual void gc_blacken(VM& vm) {}

  StringObject* as_string();
  const char* as_cstring();
  NativeObject* as_native();
  FunctionObject* as_function();
  UpvalueObject* as_upvalue();
  ClosureObject* as_closure();
};

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean{};
    double numeric;
    BaseObject* object;
  } as_;
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}

  str_t stringify() const { return "value"; }
};

inline std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val.stringify();
}

}