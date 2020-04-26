#include <iostream>
#include <InfantGC/Object.hh>
#include <InfantGC/VM.hh>

namespace _mevo::infant {

VM::VM() noexcept {
}

VM::~VM() {
  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();
    reclaim_object(o);
  }
}

void VM::reclaim_object(BaseObject* o) {
  std::cout << "reclaim object `" << object_asstr(o) << "`" << std::endl;
  delete o;
}

void VM::append_to_roots(BaseObject* o) {
  if (objects_.size() > kGCThreshold)
    collect();

  objects_.push_back(o);
}

void VM::mark_object(BaseObject* o) {
  if (o == nullptr || o->marked())
    return;

  o->set_marked(true);
  worklist_.push_back(o);
}

void VM::push(BaseObject* o) {
  roots_.push_back(o);
}

BaseObject* VM::pop() {
  if (roots_.empty())
    return nullptr;

  auto* o = roots_.back();
  roots_.pop_back();
  return o;
}

BaseObject* VM::peek(sz_t distance) const {
  if (roots_.size() > distance)
    return roots_[roots_.size() - 1 - distance];
  return nullptr;
}

void VM::collect() {
  std::cout << "*** collect starting ***" << std::endl;

  // mark roots
  for (auto* o : roots_)
    mark_object(o);

  while (!worklist_.empty()) {
    auto* o = worklist_.back();
    worklist_.pop_back();
    o->blacken(*this);
  }

  // sweep
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->marked()) {
      reclaim_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(false);
      ++it;
    }
  }

  std::cout << "*** collect finished *** [" << objects_.size() << "] objects alived" << std::endl;
}

}