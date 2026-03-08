#pragma once

#include <memory>
#include <string>

namespace ms {

class RuntimeObject {
 public:
  virtual ~RuntimeObject() = default;
  virtual std::string ToString() const = 0;
};

struct StringObject : public RuntimeObject {
  explicit StringObject(std::string v) : value(std::move(v)) {}
  std::string ToString() const override { return value; }
  std::string value;
};

std::shared_ptr<StringObject> MakeStringObject(const std::string& value);

}  // namespace ms
