#include "runtime/object.hh"

namespace ms {

std::shared_ptr<StringObject> MakeStringObject(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms
