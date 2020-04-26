#pragma once

#include <InfantGC/Object.hh>

namespace _mevo::infant {

class PairObject final : public BaseObject {
  BaseObject* first_{};
  BaseObject* second_{};
public:
  PairObject(BaseObject* first, BaseObject* second) noexcept
    : BaseObject(ObjType::PAIR)
    , first_(first)
    , second_(second) {
  }

  inline BaseObject* first() const noexcept { return first_; }
  inline void set_first(BaseObject* first) noexcept { first_ = first; }
  inline BaseObject* second() const noexcept { return second_; }
  inline void set_second(BaseObject* second) noexcept { second_ = second; }

  virtual str_t stringify() const override;
  virtual void blacken(VM& vm) override;
  static PairObject* create(VM& vm, BaseObject* first, BaseObject* second);
};

}