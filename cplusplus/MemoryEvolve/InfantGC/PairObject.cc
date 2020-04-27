#include <InfantGC/VM.hh>
#include <InfantGC/PairObject.hh>

namespace _mevo::infant {

str_t PairObject::stringify() const {
  ss_t ss;
  ss << "Pair{`0x" << this << "`, (`0x" << first_ << "`, `0x" << second_ << "`)}";
  return ss.str();
}

void PairObject::blacken(VM& vm) {
  vm.mark_object(first_);
  vm.mark_object(second_);
}

PairObject* PairObject::create(VM& vm, BaseObject* first, BaseObject* second) {
  return create_object<PairObject>(vm, first, second);
}

}