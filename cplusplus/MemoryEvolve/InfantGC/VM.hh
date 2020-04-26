#pragma once

#include <list>
#include <vector>
#include <Core/MemoryEvolve.hh>

namespace _mevo::infant {

class BaseObject;

class VM final : private UnCopyable {
  static constexpr sz_t kGCThreshold = 1 << 5;

  std::vector<BaseObject*> roots_;
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> worklist_;

  VM() noexcept;
  ~VM();

  void reclaim_object(BaseObject* o);
public:
  static VM& get_instance() {
    static VM _ins;
    return _ins;
  }

  void append_to_roots(BaseObject* o);
  void mark_object(BaseObject* o);

  void push(BaseObject* o);
  BaseObject* pop();
  BaseObject* peek(sz_t distance = 0) const;

  void collect();
};

template <typename T, typename... Args>
inline T* create_object(VM& vm, Args&&... args) {
  auto* o = new T(std::forward<Args>(args)...);
  vm.append_to_roots(o);
  return o;
}

}