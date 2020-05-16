#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include "common.hh"
#include "value.hh"

namespace tadpole {

enum class InterpretRet {
  OK,
  ECOMPILE, // compile error
  ERUNTIME, // runtime error
};

class CallFrame;
class TadpoleCompiler;

class VM final : private UnCopyable {
  static constexpr sz_t kGCThreshold = 1 << 10;
  static constexpr sz_t kGCFactor = 2;
  static constexpr sz_t kDefaultCap = 256;

  TadpoleCompiler* tcompiler_{};
  bool is_running_{true};

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  std::unordered_map<strv_t, Value> globals_;
  std::unordered_map<u32_t, StringObject*> interned_strings_;
  UpvalueObject* open_upvalues_{};

  sz_t gc_threshold_{kGCThreshold};
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> worklist_;

  void reset();
  void runtime_error(const char* format, ...);

  void push(Value value) noexcept;
  Value pop() noexcept;
  const Value& peek(sz_t distance = 0) const noexcept;

  bool call(ClosureObject* closure, sz_t argc);
  bool call(const Value& callee, sz_t argc);

  void collect();
  void reclaim_object(BaseObject* o);
public:
  VM() noexcept;
  ~VM();

  void define_native(const str_t& name, NativeFn&& fn);

  void append_object(BaseObject* o);
  void mark_object(BaseObject* o);
  void mark_value(const Value& v);

  inline bool is_running() const noexcept { return is_running_; }
  inline void terminate() noexcept { is_running_ = false; }

  inline void set_interned(u32_t h, StringObject* s) noexcept {
    interned_strings_[h] = s;
  }

  inline StringObject* get_interned(u32_t h) const noexcept {
    if (auto it = interned_strings_.find(h); it != interned_strings_.end())
      return it->second;
    return nullptr;
  }
};

}