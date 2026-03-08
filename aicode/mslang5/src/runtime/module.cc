#include "runtime/module.hh"

#include <filesystem>

#include "support/source.hh"
#include "runtime/vm.hh"

namespace ms {

ModuleLoader::ModuleLoader() { search_paths_.push_back("."); }

void ModuleLoader::AddSearchPath(std::string path) {
  search_paths_.push_back(std::move(path));
}

std::optional<std::string> ModuleLoader::ResolvePath(
    const std::string& module_name) const {
  std::string relative = module_name;
  for (char& c : relative) {
    if (c == '.') {
      c = '/';
    }
  }
  relative += ".ms";

  for (const std::string& root : search_paths_) {
    std::filesystem::path p = std::filesystem::path(root) / relative;
    if (std::filesystem::exists(p)) {
      return p.string();
    }
  }
  return std::nullopt;
}

std::shared_ptr<Module> ModuleLoader::Load(const std::string& module_name, Vm& vm,
                                           std::string* error) {
  if (const auto it = cache_.find(module_name); it != cache_.end()) {
    if (it->second->initializing) {
      if (error != nullptr) {
        *error = "circular module dependency detected: " + module_name;
      }
      return nullptr;
    }
    return it->second;
  }

  auto path = ResolvePath(module_name);
  if (!path) {
    if (error != nullptr) {
      *error = "module not found: " + module_name;
    }
    return nullptr;
  }

  auto source = SourceFile::LoadFromPath(*path);
  if (!source) {
    if (error != nullptr) {
      *error = source.error();
    }
    return nullptr;
  }

  auto module = std::make_shared<Module>();
  module->name = module_name;
  module->initializing = true;
  cache_[module_name] = module;

  std::string runtime_error;
  const InterpretResult r = vm.ExecuteModule(source->Text(), module, &runtime_error);
  if (r != InterpretResult::kOk) {
    cache_.erase(module_name);
    if (error != nullptr) {
      *error = "failed to load module '" + module_name + "': " + runtime_error;
    }
    return nullptr;
  }
  module->initializing = false;
  module->initialized = true;
  return module;
}

}  // namespace ms

