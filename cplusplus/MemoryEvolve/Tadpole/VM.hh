#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include <Tadpole/Value.hh>

namespace _mevo::tadpole {

class CallFrame;
class GlobalCompiler;

enum class InterpretRet {
  OK,
  ECOMPILE, // compile error
  ERUNTIME, // runtime error
};

class VM final : private UnCopyable {
  static constexpr sz_t kGCThreshold = 1 << 10;
  static constexpr sz_t kGCFactor = 2;
  static constexpr sz_t kDefaultCap = 256;

  GlobalCompiler* gcompiler_{};
  bool running_{true};

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  std::unordered_map<str_t, Value> globals_;
  std::unordered_map<u32_t, StringObject*> interned_strings_;
  UpvalueObject* open_upvalues_{};

  sz_t gc_threshold_{kGCThreshold};
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> worklist_;

  void reset();
  void runtime_error(const char* fromat, ...);

  void push(Value value) noexcept;
  Value pop() noexcept;
  const Value& peek(int distance = 0) const noexcept;

  bool call(ClosureObject* closure, int argc);
  bool call(Value callee, int argc);

  UpvalueObject* capture_upvalue(Value* local);
  void close_upvalues(Value* last);

  InterpretRet run();

  void collect();
  void reclaim_object(BaseObject* o);
public:
  VM() noexcept;
  ~VM();

  void define_native(const str_t& name, NativeFn&& fn);
  InterpretRet interpret(const str_t& source_bytes);

  void append_object(BaseObject* o);
  void mark_object(BaseObject* o);
  void mark_value(const Value& v);

  inline bool is_running() const noexcept { return running_; }
  inline void stop_vm() noexcept { running_ = false; }

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