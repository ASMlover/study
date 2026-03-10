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
  explicit StringObject(std::string v) : value(std::move(v)) {}
  std::string to_string() const override { return value; }
  std::string value;
};

std::shared_ptr<StringObject> make_string_object(const std::string& value);

}  // namespace ms
