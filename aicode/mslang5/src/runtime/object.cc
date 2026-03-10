#include "runtime/object.hh"

namespace ms {

std::shared_ptr<StringObject> make_string_object(const std::string& value) {
  return std::make_shared<StringObject>(value);
}

}  // namespace ms
