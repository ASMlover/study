#pragma once

#include <string_view>

bool register_task(std::string_view name, void (*closure)());
int run_all_tasks();

#define TASK_DECL(Name)\
class _ConcurrencyTask_##Name final {\
public:\
  void _run();\
  static void _run_task() {\
    _ConcurrencyTask_##Name t;\
    t._run();\
  }\
};\
bool _ConcurrencyTask_##Name_ignored = register_task(#Name, &_ConcurrencyTask_##Name::_run_task);\
void _ConcurrencyTask_##Name::_run()