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
  inline explicit ClosureObject(std::shared_ptr<FunctionObject> fn)
      : function(std::move(fn)) {}
  inline std::string to_string() const override {
    if (function == nullptr) {
      return "<closure null>";
    }
    return function->to_string();
  }

  std::shared_ptr<FunctionObject> function;
  std::vector<std::shared_ptr<UpvalueObject>> upvalues;
};

struct ClassObject : public RuntimeObject {
  explicit ClassObject(std::string n, std::shared_ptr<ClassObject> parent = nullptr)
      : name(std::move(n)), superclass(std::move(parent)) {}

  inline std::string to_string() const override { return "<class " + name + ">"; }

  std::string name;
  std::shared_ptr<ClassObject> superclass;
  Table methods;
};

struct InstanceObject : public RuntimeObject {
  explicit InstanceObject(std::shared_ptr<ClassObject> k) : klass(std::move(k)) {}

  inline std::string to_string() const override {
    const std::string class_name = klass != nullptr ? klass->name : "unknown";
    return "<" + class_name + " instance>";
  }

  std::shared_ptr<ClassObject> klass;
  Table fields;
};

struct BoundMethodObject : public RuntimeObject {
  BoundMethodObject(Value recv, std::shared_ptr<ClosureObject> m)
      : receiver(std::move(recv)), method(std::move(m)) {}

  inline std::string to_string() const override {
    if (method == nullptr) {
      return "<bound method:null>";
    }
    return "<bound " + method->to_string() + ">";
  }

  Value receiver;
  std::shared_ptr<ClosureObject> method;
};

inline std::shared_ptr<StringObject> make_string_object(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms
