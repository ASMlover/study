module;

#include <functional>
#include <iostream>
#include "common.hh"

import common;

export module harness;

export namespace tadpole::harness {

class Tester final : private UnCopyable {
  bool ok_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept : fname_(fname), lineno_(lineno) {}

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr << fname_ << "(" << lineno_ << "): " << ss_.str() << std::endl;
      std::exit(-1);
    }
  }

  inline Tester& is_true(bool cond, strv_t msg) noexcept {
    if (!cond) {
      ss_ << "Assertion failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define _HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y> inline Tester& Name(const X& x, const Y& y) noexcept {\
    if (!(x Op y)) {\
      ss_ << " failed: " << x << (" " #Op " ") << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  _HARNESS_BINOP(is_eq, ==)
  _HARNESS_BINOP(is_ne, !=)
  _HARNESS_BINOP(is_gt, >)
  _HARNESS_BINOP(is_ge, >=)
  _HARNESS_BINOP(is_lt, <)
  _HARNESS_BINOP(is_le, <=)
#undef _HARNESS_BINOP
};

}