#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bytecode/chunk.hh"
#include "runtime/value.hh"

namespace ms {

class RuntimeObject {
 public:
  virtual ~RuntimeObject() = default;
  virtual std::string to_string() const = 0;
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

inline std::shared_ptr<StringObject> make_string_object(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms