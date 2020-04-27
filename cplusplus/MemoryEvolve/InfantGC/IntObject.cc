#include <InfantGC/VM.hh>
#include <InfantGC/IntObject.hh>

namespace _mevo::infant {

str_t IntObject::stringify() const {
  ss_t ss;
  ss << "Int{`0x" << this << "`, " << value_ << "}";
  return ss.str();
}

IntObject* IntObject::create(VM& vm, int value) {
  return create_object<IntObject>(vm, value);
}

}