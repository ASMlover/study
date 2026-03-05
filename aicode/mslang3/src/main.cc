#include "common.hh"
#include "logger.hh"

int main(int argc, char* argv[]) {
    ms::Logger::info("Maple Script Language v0.1.0");
    ms::Logger::debug("Debug mode enabled");
    if (argc > 1) {
        ms::Logger::info(std::string("Running script: ") + argv[1]);
    } else {
        ms::Logger::warn("No script provided, starting REPL...");
        ms::Logger::error("REPL not implemented yet");
    }
    return 0;
}
