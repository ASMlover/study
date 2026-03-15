#include "runtime/value.hh"

#include <sstream>
#include <utility>

#include "runtime/object.hh"

namespace ms {

Value::Value(bool v) : storage_(v) {}

Value::Value(double v) : storage_(v) {}

Value::Value(std::string v) : storage_(std::move(v)) {}

Value::Value(std::shared_ptr<Module> module) : storage_(std::move(module)) {}

Value::Value(RuntimeObject* object) : storage_(object) {}

Value Value::nil() noexcept { return Value(); }

bool Value::is_nil() const noexcept { return std::holds_alternative<std::monostate>(storage_); }

bool Value::is_bool() const noexcept { return std::holds_alternative<bool>(storage_); }

bool Value::is_number() const noexcept { return std::holds_alternative<double>(storage_); }

bool Value::is_string() const noexcept { return std::holds_alternative<std::string>(storage_); }

bool Value::is_module() const noexcept {
  return std::holds_alternative<std::shared_ptr<Module>>(storage_);
}

bool Value::is_object() const noexcept {
  return std::holds_alternative<RuntimeObject*>(storage_);
}

bool Value::as_bool() const { return std::get<bool>(storage_); }

double Value::as_number() const { return std::get<double>(storage_); }

const std::string& Value::as_string() const { return std::get<std::string>(storage_); }

std::shared_ptr<Module> Value::as_module() const {
  return std::get<std::shared_ptr<Module>>(storage_);
}

RuntimeObject* Value::as_object() const {
  return std::get<RuntimeObject*>(storage_);
}

std::string Value::to_string() const {
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

bool operator==(const Value& lhs, const Value& rhs) { return lhs.storage_ == rhs.storage_; }

}  // namespace ms