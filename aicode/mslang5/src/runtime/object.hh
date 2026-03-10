#pragma once

#include <memory>
#include <string>

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

inline std::shared_ptr<StringObject> make_string_object(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms
