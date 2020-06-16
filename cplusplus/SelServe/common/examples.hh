#pragma once

#include <functional>
#include "common.hh"

namespace sser::examples {

using ExampleFn = std::function<void()>;

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleFn&& fn);
int launch_examples(strv_t alias);

}

#if defined(SSER_RUN_EXAMPLES)
# define _SSER_IGNORED_EXAMPLE(Name, Alias, Doc, Fn)\
  bool _Ignored_SSerExample_##Name = sser::examples::register_example(#Name, #Alias, Doc, Fn);
#else
# define _SSER_IGNORED_EXAMPLE(Name, Alias, Doc, Fn)
#endif

#define SSER_EXAMPLE(Name, Alias, Doc)\
class SSerExample_##Name final : private sser::UnCopyable {\
  void _run();\
public:\
  static void _run_example() {\
    static SSerExample_##Name _ins;\
    _ins._run();\
  }\
};\
_SSER_IGNORED_EXAMPLE(Name, Alias, Doc, &SSerExample_##Name::_run_example)\
void SSerExample_##Name::_run()