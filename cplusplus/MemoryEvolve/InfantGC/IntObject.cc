#include <InfantGC/VM.hh>
#include <InfantGC/IntObject.hh>

namespace _mevo::infant {

str_t IntObject::stringify() const {
  return std::to_string(value_);
}

IntObject* IntObject::create(VM& vm, int value) {
  return create_object<IntObject>(vm, value);
}

}