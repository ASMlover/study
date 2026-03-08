#pragma once

#include <string>

namespace ms {

class Vm;

class ScriptInterpreter {
 public:
  // Transitional non-normative path kept only for migration/debug.
  static bool Execute(Vm& vm, const std::string& source, std::string* error);
  static bool IsCompileLikeError(const std::string& error);
};

}  // namespace ms
