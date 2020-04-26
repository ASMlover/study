#pragma once

#include <Core/MemoryEvolve.hh>

namespace _mevo::infant {

class VM;

enum class ObjType {
  INT,
  PAIR,
};

class BaseObject : private UnCopyable {
  bool marked_{};
  ObjType type_;
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline bool marked() const noexcept { return marked_; }
  inline void set_marked(bool marked = true) noexcept { marked_ = marked; }
  inline ObjType type() const noexcept { return type_; }

  virtual str_t stringify() const { return "<Object>"; }
  virtual void blacken(VM& vm) {}
};

inline str_t object_asstr(BaseObject* o) noexcept {
  return o != nullptr ? o->stringify() : "<Nil>";
}

}