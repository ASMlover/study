#include <iostream>
#include "chunk.hh"
#include "compiler.hh"
#include "vm.hh"

namespace tadpole {

class CallFrame {
};

VM::VM() noexcept {
  tcompiler_ = new TadpoleCompiler();
  stack_.reserve(kDefaultCap);
}

VM::~VM() {
  delete tcompiler_;

  globals_.clear();
  interned_strings_.clear();

  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();
    reclaim_object(o);
  }
}

void VM::append_object(BaseObject* o) {
  if (objects_.size() >= kGCThreshold)
    collect();

  objects_.push_back(o);
}

void VM::mark_object(BaseObject* o) {
  if (o == nullptr || o->is_marked())
    return;

#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] mark object: `" << o->stringify() << "`" << std::endl;
#endif

  o->set_marked(true);
  worklist_.push_back(o);
}

void VM::mark_value(const Value& v) {
  if (v.is_object())
    mark_object(v.as_object());
}

void VM::collect() {}

void VM::reclaim_object(BaseObject* o) {
#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] reclaim object: `" << o->stringify() << "`" << std::endl;
#endif

  delete o;
}

}