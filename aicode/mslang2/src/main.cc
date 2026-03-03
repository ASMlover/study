#include "ms/scanner.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace ms {

void printToken(const Token& token) {
    std::cout << "Token{type=" << static_cast<int>(token.type)
              << ", text=\"" << token.start << "\""
              << ", line=" << token.line << "}" << std::endl;
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

    Scanner scanner;
    scanner.initSource(source);

    Token token;
    while (true) {
        token = scanner.scanToken();
        printToken(token);
        if (token.type == TokenType::TOKEN_EOF) {
            break;
        }
    }
}

void runPrompt() {
    Scanner scanner;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) {
            continue;
        }

        scanner.initSource(line);

        Token token;
        while (true) {
            token = scanner.scanToken();
            printToken(token);
            if (token.type == TokenType::TOKEN_EOF) {
                break;
            }
        }
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
