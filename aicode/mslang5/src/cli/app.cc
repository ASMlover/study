#include "cli/app.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "runtime/module.hh"
#include "runtime/vm.hh"

namespace ms {

namespace {

std::string ReadFile(const std::string& path, std::string* error) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    if (error != nullptr) {
      *error = "failed to open file: " + path;
    }
    return {};
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

int RunFile(const std::string& path) {
  std::string error;
  const std::string source = ReadFile(path, &error);
  if (!error.empty()) {
    std::cerr << error << '\n';
    return 1;
  }
  Vm vm;
  const std::filesystem::path p(path);
  vm.Modules().AddSearchPath(p.parent_path().string());
  const InterpretResult r = vm.ExecuteSource(source, &error);
  if (r != InterpretResult::kOk) {
    std::cerr << error << '\n';
    return static_cast<int>(r);
  }
  return 0;
}

int RunRepl() {
  Vm vm;
  std::string line;
  while (true) {
    std::cout << "maple> ";
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (line == ":quit" || line == ":exit") {
      break;
    }
    std::string error;
    const InterpretResult r = vm.ExecuteSource(line + "\n", &error);
    if (r != InterpretResult::kOk) {
      std::cerr << error << '\n';
    }
  }
  return 0;
}

}  // namespace

int run_cli(int argc, char** argv) {
  if (argc <= 1) {
    return RunRepl();
  }
  return RunFile(argv[1]);
}

}  // namespace ms
