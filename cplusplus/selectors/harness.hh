#pragma once

#include <functional>
#include <iostream>
#include "common.hh"

namespace sel::harness {

class Tester final : private UnCopyable {
  bool ok_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept
    : fname_(fname), lineno_(lineno) {
  }

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr << fname_ << "(" << lineno_ << "): " << ss_.str() << std::endl;
      std::exit(-1);
    }
  }

  inline Tester& is_true(bool cond, strv_t msg) noexcept {
    if (!cond) {
      ss_ << " Assertion failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define _HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y>\
  inline Tester& Name(const X& x, const Y& y) noexcept {\
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

  template <typename T>
  inline Tester& operator<<(const T& x) noexcept {
    if (!ok_)
      ss_ << " " << x;
    return *this;
  }
};

using HarnessClosure = std::function<void ()>;
bool register_harness(strv_t name, HarnessClosure&& closure) noexcept;
int run_all_harness();

}

#define SEL_CHECK_TRUE(c)   sel::harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define SEL_CHECK_EQ(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define SEL_CHECK_NE(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define SEL_CHECK_GT(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define SEL_CHECK_GE(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define SEL_CHECK_LT(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_lt((a), (b))
#define SEL_CHECK_LE(a, b)  sel::harness::Tester(__FILE__, __LINE__).is_le((a), (b))

#if defined(_SEL_RUN_HARNESS)
# define _SEL_IGNORED_REG(Name, Fn) bool _Ignored_SelHarness_##Name = _sel::harness::register_harness(#Name, Fn);
#else
# define _SEL_IGNORED_REG(Name, Fn)
#endif

#define SEL_TEST(Name)\
class SelHarness_##Name final : private sel::UnCopyable {\
  void _run();\
public:\
  static void _run_harness() {\
    static SelHarness_##Name _ins;\
    _ins._run();\
  }\
};\
_SEL_IGNORED_REG(Name, &SelHarness_##Name::_run_harness)\
void SelHarness_##Name::_run()