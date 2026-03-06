#include "ms/vm.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace ms {

void runFile(const std::string& path, VM& vm) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << std::endl;
        return;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string source = oss.str();

    vm.interpret(source);
}

void runPrompt(VM& vm) {
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) {
            continue;
        }

        vm.interpret(line);
    }
    std::cout << std::endl;
}

}

int main(int argc, char* argv[]) {
    ms::VM vm;

    if (argc > 1) {
        ms::runFile(argv[1], vm);
    } else {
        ms::runPrompt(vm);
    }

    return 0;
}
