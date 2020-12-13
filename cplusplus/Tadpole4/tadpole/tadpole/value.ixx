module;
#include <functional>
#include "common.hh"
import common;

export module value;

export namespace tadpole {

enum class ObjType : u8_t {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
};

class VM;

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
  inline bool is_marked() const noexcept { return marked_; }
  inline void set_marked(bool marked = true) noexcept { marked_ = marked; }

  virtual bool is_truthy() const { return true; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void gc_blacken(VM& vm) {}

  StringObject* as_string();
  const char* as_cstring();
  NativeObject* as_native();
  FunctionObject* as_function();
  UpvalueObject* as_upvalue();
  ClosureObject* as_closure();
};

enum class ValueType : u8_t {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* object{};
  } as_;

  template <typename T> inline void set_numeric(T x) noexcept { as_.numeric = as_type<double>(x); }
  // inline bool is(ObjType type) const noexcept { return }
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
};

StringObject* BaseObject::as_string() { return nullptr; }
const char* BaseObject::as_cstring() { return nullptr; }
NativeObject* BaseObject::as_native() { return nullptr; }
FunctionObject* BaseObject::as_function() { return nullptr; }
UpvalueObject* BaseObject::as_upvalue() { return nullptr; }
ClosureObject* BaseObject::as_closure() { return nullptr; }

}