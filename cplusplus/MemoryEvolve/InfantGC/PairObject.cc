#include <InfantGC/VM.hh>
#include <InfantGC/PairObject.hh>

namespace _mevo::infant {

str_t PairObject::stringify() const {
  ss_t ss;
  ss << "{first<" << first_ << ">, second<" << second_ << ">}";
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