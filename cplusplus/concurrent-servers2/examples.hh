#pragma once

#include "common.hh"

namespace coext {

class FakeExample {};
using ExampleClosure = std::function<void ()>;

bool register_example(strv_t name, strv_t alias, ExampleClosure&& closure);
int launch_examples(strv_t name);

}

#define COEXT_EXAMPLE(Name, Alias)\
class _CoextExample_##Name final : public coext::FakeExample {\
  void _run();\
public:\
  static void _run_example() {\
    _CoextExample_##Name e;\
    e._run();\
  }\
};\
bool _Ignored_CoextExample_##Name =\
  coext::register_example(#Name, #Alias, &_CoextExample_##Name::_run_example);\
void _CoextExample_##Name::_run()