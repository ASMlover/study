#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "runtime/table.hh"

namespace ms {

class Vm;

struct Module {
  std::string name;
  Table exports;
  bool initializing = false;
  bool initialized = false;
};

class ModuleLoader {
 public:
  ModuleLoader();

  void AddSearchPath(std::string path);
  std::optional<std::string> ResolvePath(const std::string& module_name) const;

  std::shared_ptr<Module> Load(const std::string& module_name, Vm& vm,
                               std::string* error);

 private:
  std::vector<std::string> search_paths_;
  std::unordered_map<std::string, std::shared_ptr<Module>> cache_;
  std::unordered_map<std::string, std::string> failed_;
};

}  // namespace ms
