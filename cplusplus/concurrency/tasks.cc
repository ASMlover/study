#include <iostream>
#include <memory>
#include <vector>
#include "tasks.hh"

struct TaskContext {
  std::string_view name{};
  void (*closure)();

  TaskContext(std::string_view n, void (*fn)()) noexcept
    : name(n), closure(fn) {
  }
};

std::vector<TaskContext> g_tasks;

bool register_task(std::string_view name, void (*closure)()) {
  g_tasks.push_back(TaskContext(name, closure));
  return true;
}

int run_all_tasks() {
  for (auto& t : g_tasks) {
    std::cout << "========= RUN TASK [" << t.name << "] =========" << std::endl;
    t.closure();
    std::cout << std::endl;
  }

  return 0;
}