#pragma once

#include <memory>
#include <string>
#include <variant>

namespace ms {

struct Module;
class RuntimeObject;

class Value {
 public:
  using Storage = std::variant<std::monostate, bool, double, std::string,
                               std::shared_ptr<Module>, RuntimeObject*>;

  Value() = default;
  explicit Value(bool v);
  explicit Value(double v);
  explicit Value(std::string v);
  explicit Value(std::shared_ptr<Module> module);
  explicit Value(RuntimeObject* object);

  static Value nil() noexcept;

  bool is_nil() const noexcept;
  bool is_bool() const noexcept;
  bool is_number() const noexcept;
  bool is_string() const noexcept;
  bool is_module() const noexcept;
  bool is_object() const noexcept;

  bool as_bool() const;
  double as_number() const;
  const std::string& as_string() const;
  std::shared_ptr<Module> as_module() const;
  RuntimeObject* as_object() const;

  std::string to_string() const;

  friend bool operator==(const Value& lhs, const Value& rhs);

 private:
  Storage storage_;
};

bool operator==(const Value& lhs, const Value& rhs);

}  // namespace ms