#pragma once

#include <functional>
#include <iostream>
#include <Core/MemoryEvolve.hh>

namespace _mevo {

class FakeTester {};

class Tester final : public Copyable {
  bool ok_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept
    : fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr << fname_ << ":" << lineno_ << " - " << ss_.str() << std::endl;
      std::exit(-1);
    }
  }

  Tester& is_true(bool cond, strv_t msg) noexcept {
    if (!cond) {
      ss_ << " Assertion failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define _MEVO_HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y>\
  Tester& Name(const X& x, const Y& y) noexcept {\
    if (!(x Op y)) {\
      ss_ << " failed: " << x << (" " #Op " ") << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  _MEVO_HARNESS_BINOP(is_eq, ==)
  _MEVO_HARNESS_BINOP(is_ne, !=)
  _MEVO_HARNESS_BINOP(is_ge, >=)
  _MEVO_HARNESS_BINOP(is_gt, >)
  _MEVO_HARNESS_BINOP(is_le, <=)
  _MEVO_HARNESS_BINOP(is_lt, <)
#undef _MEVO_HARNESS_BINOP

  template <typename Value>
  inline Tester& operator<<(const Value& value) noexcept {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

using HarnessClosure = std::function<void ()>;
bool register_harness(strv_t base, strv_t name, HarnessClosure&& closure);
int run_all_harness();

}

#define _MEVO_CHECK_TRUE(c)   _mevo::Tester(__FILE__, __LINE__).is_true((c), #c)
#define _MEVO_CHECK_EQ(a, b)  _mevo::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define _MEVO_CHECK_NE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define _MEVO_CHECK_GE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define _MEVO_CHECK_GT(a, b)  _mevo::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define _MEVO_CHECK_LE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_le((a), (b))
#define _MEVO_CHECK_LT(a, b)  _mevo::Tester(__FILE__, __LINE__).is_lt((a), (b))

#define _MEVO_TEST(Name, Base)\
class _MevoHarness_##Name final : public Base {\
  void _run();\
public:\
  static void _run_harness() {\
    _MevoHarness_##Name _ins;\
    _ins._run();\
  }\
};\
bool _Ignored_MevoHarness_##Name =\
  _mevo::register_harness(#Base, #Name, &_MevoHarness_##Name::_run_harness);\
void _MevoHarness_##Name::_run()