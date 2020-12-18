#pragma once

#include <functional>
#include "common.hh"

namespace tadpole {

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

}