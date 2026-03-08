#pragma once

#include <memory>
#include <string>

namespace ms {

struct StringObject {
  explicit StringObject(std::string v) : value(std::move(v)) {}
  std::string value;
};

std::shared_ptr<StringObject> MakeStringObject(const std::string& value);

}  // namespace ms

