#pragma once

#include <functional>
#include <iostream>
#include "common.hh"

namespace tadpole::harness {

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

using HarnessClosure = std::function<void()>;
bool register_harness(strv_t name, HarnessClosure&& closure);
int run_all_harness();

}

#define TADPOLE_CHECK_TRUE(c)   tadpole::harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define TADPOLE_CHECK_EQ(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define TADPOLE_CHECK_NE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define TADPOLE_CHECK_GT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define TADPOLE_CHECK_GE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define TADPOLE_CHECK_LT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_lt((a), (b))
#define TADPOLE_CHECK_LE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_le((a), (b))

#if defined(_TADPOLE_RUN_HARNESS)
# define _TADPOLE_IGNORED_REG(N, Fn) bool _Ignored_TadpoleHarness_##N = tadpole::harness::register_harness(#N, Fn);
#else
# define _TADPOLE_IGNORED_REG(N, Fn)
#endif

#define TADPOLE_TEST(Name)\
class TadpoleHarness_##Name final : private tadpole::UnCopyable {\
  void _run();\
public:\
  static void _run_harness() {\
    static TadpoleHarness_##Name _ins;\
    _ins._run();\
  }\
};\
_TADPOLE_IGNORED_REG(Name, &TadpoleHarness_##Name::_run_harness)\
void TadpoleHarness_##Name::_run()
