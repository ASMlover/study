#pragma once

#include <string>

namespace ms {

class Vm;

class ScriptInterpreter {
 public:
  // Transitional non-normative path kept only for migration/debug.
  static bool execute(Vm& vm, const std::string& source, std::string* error);
  static bool is_compile_like_error(const std::string& error);
};

}  // namespace ms
