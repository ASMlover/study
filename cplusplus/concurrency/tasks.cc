#include <iostream>
#include <memory>
#include <vector>
#include "tasks.hh"

namespace task {

struct TaskContext {
  std::string_view name{};
  void (*closure)();

  TaskContext(std::string_view n, void (*fn)()) noexcept
    : name(n), closure(fn) {
  }
};

using TaskContextVector = std::vector<TaskContext>;
TaskContextVector* g_tasks{};

bool register_task(std::string_view name, void (*closure)()) {
  if (!g_tasks)
    g_tasks = new TaskContextVector;

  g_tasks->push_back(TaskContext(name, closure));
  return true;
}

int run_all_tasks() {
  if (!g_tasks || g_tasks->empty())
    return 0;

  for (auto& t : *g_tasks) {
    std::cout << "========= RUN TASK [" << t.name << "] =========" << std::endl;
    t.closure();
    std::cout << std::endl;
  }

  return 0;
}

}
