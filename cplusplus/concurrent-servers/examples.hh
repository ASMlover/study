#pragma once

#include <functional>
#include <string_view>

namespace efw {
// examples framework

class FakeExample {};
using ClosureFn = std::function<void ()>;

bool register_example(std::string_view name, ClosureFn&& closure);
int launch_all_examples();

}

#define EFW_EXAMPLE(Name)\
class _Example_##Name final : public efw::FakeExample {\
  void _run();\
public:\
  static void _run_example() {\
    _Example_##Name e;\
    e._run();\
  }\
};\
bool _Ignored_Example_##Name = efw::register_example(#Name, &_Example_##Name::_run_example);\
void _Example_##Name::_run()