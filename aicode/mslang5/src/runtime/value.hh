#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <variant>

#include "runtime/object.hh"

namespace ms {

struct Module;

class Value {
 public:
  using Storage = std::variant<std::monostate, bool, double, std::string,
                               std::shared_ptr<Module>, std::shared_ptr<RuntimeObject>>;

  Value() = default;
  explicit Value(bool v) : storage_(v) {}
  explicit Value(double v) : storage_(v) {}
  explicit Value(std::string v) : storage_(std::move(v)) {}
  explicit Value(std::shared_ptr<Module> module) : storage_(std::move(module)) {}
  explicit Value(std::shared_ptr<RuntimeObject> object) : storage_(std::move(object)) {}

  static Value nil() noexcept { return Value(); }

  bool is_nil() const noexcept { return std::holds_alternative<std::monostate>(storage_); }
  bool is_bool() const noexcept { return std::holds_alternative<bool>(storage_); }
  bool is_number() const noexcept { return std::holds_alternative<double>(storage_); }
  bool is_string() const noexcept { return std::holds_alternative<std::string>(storage_); }
  bool is_module() const noexcept {
    return std::holds_alternative<std::shared_ptr<Module>>(storage_);
  }
  bool is_object() const noexcept {
    return std::holds_alternative<std::shared_ptr<RuntimeObject>>(storage_);
  }

  bool as_bool() const { return std::get<bool>(storage_); }
  double as_number() const { return std::get<double>(storage_); }
  const std::string& as_string() const { return std::get<std::string>(storage_); }
  std::shared_ptr<Module> as_module() const {
    return std::get<std::shared_ptr<Module>>(storage_);
  }
  std::shared_ptr<RuntimeObject> as_object() const {
    return std::get<std::shared_ptr<RuntimeObject>>(storage_);
  }

  std::string to_string() const {
    if (is_nil()) {
      return "nil";
    }
    if (is_bool()) {
      return as_bool() ? "true" : "false";
    }
    if (is_number()) {
      std::ostringstream out;
      out << as_number();
      return out.str();
    }
    if (is_string()) {
      return as_string();
    }
    if (is_object()) {
      return as_object() != nullptr ? as_object()->to_string() : "<object:null>";
    }
    return "<module>";
  }

  friend bool operator==(const Value& lhs, const Value& rhs) {
    return lhs.storage_ == rhs.storage_;
  }

 private:
  Storage storage_;
};

}  // namespace ms
