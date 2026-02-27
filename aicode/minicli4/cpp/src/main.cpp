#include "minicli4/cli.hpp"

#include <string>
#include <vector>

int main(int argc, char** argv) {
  std::vector<std::string> args;
  for (int i = 1; i < argc; i++) {
    args.emplace_back(argv[i]);
  }
  return minicli4::run_cli(args);
}
