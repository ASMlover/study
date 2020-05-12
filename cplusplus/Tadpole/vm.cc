#include <algorithm>
#include <iostream>
#include "chunk.hh"
#include "compiler.hh"
#include "vm.hh"

namespace tadpole {

class CallFrame final : private UnCopyable {
  ClosureObject* closure_{};
  const u8_t* ip_{};
  sz_t stack_begpos_{};
public:
  CallFrame() noexcept {}
  CallFrame(ClosureObject* c, const u8_t* i, sz_t begpos = 0) noexcept
    : closure_(c), ip_(i), stack_begpos_(begpos) {
  }

  inline ClosureObject* closure() const noexcept { return closure_; }
  inline FunctionObject* frame_fn() const noexcept { return closure_->fn(); }
  inline Chunk* frame_chunk() const noexcept { return frame_fn()->chunk(); }
  inline const u8_t* ip() const noexcept { return ip_; }
  inline u8_t get_ip(sz_t i) const noexcept { return ip_[i]; }
  inline u8_t inc_ip() noexcept { return *ip_++; }
  inline sz_t stack_begpos() const noexcept { return stack_begpos_; }
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

void VM::define_native(const str_t& name, NativeFn&& fn) {
  globals_[name] = NativeObject::create(*this, std::move(fn));
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

void VM::collect() {
  // mark root objects
  tcompiler_->mark_compiler();
  for (auto& v : stack_)
    mark_value(v);
  for (auto& f : frames_)
    mark_object(f.closure());
  for (auto& g : globals_)
    mark_value(g.second);
  for (auto* u = open_upvalues_; u != nullptr; u = u->next())
    mark_object(u);

  // mark referenced objects
  while (!worklist_.empty()) {
    auto* o = worklist_.back();
    worklist_.pop_back();
    o->gc_blacken(*this);
  }

  // delete unmarked interned string objects
  for (auto it = interned_strings_.begin(); it != interned_strings_.end();) {
    if (!it->second->is_marked())
      interned_strings_.erase(it++);
    else
      ++it;
  }

  // sweep and reclaim unmarked objects
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      reclaim_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(false);
      ++it;
    }
  }

  gc_threshold_ = std::max(kGCThreshold, objects_.size() * kGCFactor);
}

void VM::reclaim_object(BaseObject* o) {
#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] reclaim object: `" << o->stringify() << "`" << std::endl;
#endif

  delete o;
}

}