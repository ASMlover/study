module;

#include <functional>
#include <iostream>
#include <tuple>
#include <vector>
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

  template <typename T> inline Tester& operator<<(const T& x) noexcept {
    if (!ok_)
      ss_ << " " << x;
    return *this;
  }
};

using ClosureFn   = std::function<void ()>;
using Context     = std::tuple<strv_t, ClosureFn>;
using ContextList = std::vector<Context>;

ContextList* g_harness{};

bool register_harness(strv_t name, ClosureFn&& fn) {
  if (!g_harness)
    g_harness = new ContextList;

  g_harness->push_back({name, std::move(fn)});
  return true;
}

int run_all_harness() {
  sz_t total_tests{};
  sz_t passed_tests{};

  if (g_harness && !g_harness->empty()) {
    total_tests = g_harness->size();

    for (auto& hc : *g_harness) {
      auto [hc_name, hc_fn] = hc;
      hc_fn();
      ++passed_tests;

      std::cout
        << "********* [" << hc_name << "] test harness PASSED "
        << "(" << passed_tests << "/" << total_tests << ") "
        << "*********"
        << std::endl;
    }
  }

  std::cout
    << "========= PASSED "
    << "(" << passed_tests << "/" << total_tests << ") "
    << "test harness ========="
    << std::endl;

  return 0;
}

}