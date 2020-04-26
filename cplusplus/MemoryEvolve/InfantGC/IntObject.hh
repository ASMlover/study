#pragma once

#include <InfantGC/Object.hh>

namespace _mevo::infant {

class IntObject final : public BaseObject {
  int value_{};
public:
  IntObject(int value = 0) noexcept
    : BaseObject(ObjType::INT)
    , value_(value) {
  }

  inline int value() const noexcept { return value_; }
  inline void set_value(int value) noexcept { value_ = value; }

  virtual str_t stringify() const override;
  static IntObject* create(VM& vm, int value = 0);
};

}