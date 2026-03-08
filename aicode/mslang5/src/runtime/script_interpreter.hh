#pragma once

#include <string>

namespace ms {

class Vm;

class ScriptInterpreter {
 public:
  static bool Execute(Vm& vm, const std::string& source, std::string* error);
};

}  // namespace ms
