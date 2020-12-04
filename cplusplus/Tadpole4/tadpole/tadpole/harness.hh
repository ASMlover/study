#pragma once

#include "common.hh"
import harness;

#define TADPOLE_CHECK_TRUE(c)   tadpole::harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define TADPOLE_CHECK_EQ(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define TADPOLE_CHECK_NE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define TADPOLE_CHECK_GT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define TADPOLE_CHECK_GE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define TADPOLE_CHECK_LT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_lt((a), (b))
#define TADPOLE_CHECK_LE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_le((a), (b))

#if defined(_TADPOLE_RUN_HARNESS)
# define _TADPOLE_IGNORED_REG(N, Fn)\
  bool _Ignored_TadpoleHarness_##N = tadpole::harness::register_harness(#N, Fn);
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