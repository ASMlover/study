#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bytecode/chunk.hh"
#include "runtime/table.hh"
#include "runtime/value.hh"

namespace ms {

class RuntimeObject;

struct GcObjectHeader {
  RuntimeObject* next = nullptr;
  std::size_t bytes = 0;
  bool marked = false;
  bool tracked = false;
};

class RuntimeObject {
 public:
  virtual ~RuntimeObject() = default;
  virtual std::string to_string() const = 0;

  inline GcObjectHeader& gc_header() noexcept { return gc_header_; }
  inline const GcObjectHeader& gc_header() const noexcept { return gc_header_; }

 protected:
  RuntimeObject() = default;

 private:
  GcObjectHeader gc_header_{};
};

struct StringObject : public RuntimeObject {
  inline explicit StringObject(std::string v) : value(std::move(v)) {}
  inline std::string to_string() const override { return value; }
  std::string value;
};

struct FunctionObject : public RuntimeObject {
  inline explicit FunctionObject(std::shared_ptr<FunctionPrototype> proto)
      : prototype(std::move(proto)) {}
  inline std::string to_string() const override {
    if (prototype == nullptr || prototype->name.empty()) {
      return "<fn anonymous>";
    }
    return "<fn " + prototype->name + ">";
  }

  std::shared_ptr<FunctionPrototype> prototype;
};

struct UpvalueObject : public RuntimeObject {
  inline explicit UpvalueObject(const std::size_t idx) : stack_index(idx) {}
  inline std::string to_string() const override { return "<upvalue>"; }

  std::size_t stack_index = 0;
  Value closed = Value::nil();
  bool is_closed = false;
};

struct ClosureObject : public RuntimeObject {
  inline explicit ClosureObject(FunctionObject* fn) : function(fn) {}
  inline std::string to_string() const override {
    if (function == nullptr) {
      return "<closure null>";
    }
    return function->to_string();
  }

  FunctionObject* function = nullptr;
  std::vector<UpvalueObject*> upvalues;
};

struct ClassObject : public RuntimeObject {
  explicit ClassObject(std::string n, ClassObject* parent = nullptr)
      : name(std::move(n)), superclass(parent) {}

  inline std::string to_string() const override { return "<class " + name + ">"; }

  std::string name;
  ClassObject* superclass = nullptr;
  Table methods;
};

struct InstanceObject : public RuntimeObject {
  explicit InstanceObject(ClassObject* k) : klass(k) {}

  inline std::string to_string() const override {
    const std::string class_name = klass != nullptr ? klass->name : "unknown";
    return "<" + class_name + " instance>";
  }

  ClassObject* klass = nullptr;
  Table fields;
};

struct BoundMethodObject : public RuntimeObject {
  BoundMethodObject(Value recv, ClosureObject* m)
      : receiver(std::move(recv)), method(m) {}

  inline std::string to_string() const override {
    if (method == nullptr) {
      return "<bound method:null>";
    }
    return "<bound " + method->to_string() + ">";
  }

  Value receiver;
  ClosureObject* method = nullptr;
};

inline std::shared_ptr<StringObject> make_string_object(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms