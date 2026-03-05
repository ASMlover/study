#include "logger.hh"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace ms {

LogLevel Logger::current_level_ = LogLevel::Info;
bool Logger::ansi_enabled_ = true;

void Logger::setLevel(LogLevel level) {
    current_level_ = level;
}

LogLevel Logger::getLevel() {
    return current_level_;
}

void Logger::enableAnsiOnWindows() {
    ansi_enabled_ = true;
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

const char* Logger::getLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "\033[90m";
        case LogLevel::Debug: return "\033[36m";
        case LogLevel::Info: return "\033[32m";
        case LogLevel::Warn: return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Fatal: return "\033[35m";
        default: return "\033[0m";
    }
}

void Logger::resetColor() {
    std::cout << "\033[0m";
}

void Logger::printLog(LogLevel level, const std::string& message) {
    static bool first_call = true;
    if (first_call) {
        enableAnsiOnWindows();
        first_call = false;
    }

    std::cout << getLevelColor(level);
    std::cout << "[" << getLevelString(level) << "] ";
    std::cout << message;
    resetColor();
    std::cout << std::endl;

    if (level == LogLevel::Fatal) {
        std::abort();
    }
}

}
