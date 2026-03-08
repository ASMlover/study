#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <variant>

namespace ms {

struct Module;

class Value {
 public:
  using Storage =
      std::variant<std::monostate, bool, double, std::string, std::shared_ptr<Module>>;

  Value() = default;
  explicit Value(bool v) : storage_(v) {}
  explicit Value(double v) : storage_(v) {}
  explicit Value(std::string v) : storage_(std::move(v)) {}
  explicit Value(std::shared_ptr<Module> module) : storage_(std::move(module)) {}

  static Value Nil() { return Value(); }

  bool IsNil() const { return std::holds_alternative<std::monostate>(storage_); }
  bool IsBool() const { return std::holds_alternative<bool>(storage_); }
  bool IsNumber() const { return std::holds_alternative<double>(storage_); }
  bool IsString() const { return std::holds_alternative<std::string>(storage_); }
  bool IsModule() const {
    return std::holds_alternative<std::shared_ptr<Module>>(storage_);
  }

  bool AsBool() const { return std::get<bool>(storage_); }
  double AsNumber() const { return std::get<double>(storage_); }
  const std::string& AsString() const { return std::get<std::string>(storage_); }
  std::shared_ptr<Module> AsModule() const {
    return std::get<std::shared_ptr<Module>>(storage_);
  }

  std::string ToString() const {
    if (IsNil()) {
      return "nil";
    }
    if (IsBool()) {
      return AsBool() ? "true" : "false";
    }
    if (IsNumber()) {
      std::ostringstream out;
      out << AsNumber();
      return out.str();
    }
    if (IsString()) {
      return AsString();
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

