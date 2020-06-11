#pragma once

#include <functional>
#include "common.hh"

namespace sel::examples {

using ExampleFn = std::function<void ()>;

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleFn&& fn);
int launch_examples(strv_t alias);

}

#define SEL_EXAMPLE(Name, Alias, Doc)\
class SelExample_##Name final : private sel::UnCopyable {\
  void _run();\
public:\
  static void _run_example() {\
    static SelExample_##Name _ins;\
    _ins._run();\
  }\
};\
bool _Ignored_SelExample_##Name = sel::examples::register_example(\
  #Name, #Alias, Doc, &SelExample_##Name::_run_example);\
void SelExample_##Name::_run()