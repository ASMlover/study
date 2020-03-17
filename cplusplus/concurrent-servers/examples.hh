#pragma once

#include <functional>
#include <string_view>

namespace efw {
// examples framework

class FakeExample {};
using ClosureFn = std::function<void ()>;

bool register_example(std::string_view name, std::string_view alias, ClosureFn&& closure);
int launch_examples(std::string_view name);

}

#define EFW_EXAMPLE(Name, Alias)\
class _Example_##Name final : public efw::FakeExample {\
  void _run();\
public:\
  static void _run_example() {\
    _Example_##Name e;\
    e._run();\
  }\
};\
bool _Ignored_Example_##Name = efw::register_example(#Name, #Alias, &_Example_##Name::_run_example);\
void _Example_##Name::_run()