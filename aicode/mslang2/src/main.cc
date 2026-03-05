#include "ms/scanner.hh"
#include "ms/compiler.hh"
#include "ms/chunk.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace ms {

void runSource(const std::string& source) {
    Chunk chunk;
    Scanner scanner;
    Compiler compiler(&scanner, &chunk);

    if (!compiler.compile(source, &chunk)) {
        return;
    }

    std::cout << "Compiled successfully!" << std::endl;
    std::cout << "Constants: " << chunk.constants().size() << std::endl;
    for (size_t i = 0; i < chunk.constants().size(); ++i) {
        std::cout << "  " << i << ": " << chunk.constants()[i].toString() << std::endl;
    }
}

void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << std::endl;
        return;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string source = oss.str();

    runSource(source);
}

void runPrompt() {
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) {
            continue;
        }

        runSource(line);
    }
    std::cout << std::endl;
}

}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: maple [script]" << std::endl;
        return 1;
    }

    if (argc == 2) {
        ms::runFile(argv[1]);
    } else {
        ms::runPrompt();
    }

    return 0;
}
