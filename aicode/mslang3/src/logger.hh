#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace ms {

enum class LogLevel {
    Trace, Debug, Info, Warn, Error, Fatal
};

class Logger {
public:
    static void setLevel(LogLevel level);
    static LogLevel getLevel();

    template<typename T>
    static void trace(const T& msg) { log(LogLevel::Trace, msg); }
    template<typename T>
    static void debug(const T& msg) { log(LogLevel::Debug, msg); }
    template<typename T>
    static void info(const T& msg) { log(LogLevel::Info, msg); }
    template<typename T>
    static void warn(const T& msg) { log(LogLevel::Warn, msg); }
    template<typename T>
    static void error(const T& msg) { log(LogLevel::Error, msg); }
    template<typename T>
    static void fatal(const T& msg) { log(LogLevel::Fatal, msg); }

private:
    static LogLevel current_level_;
    static bool ansi_enabled_;

    static void enableAnsiOnWindows();
    static const char* getLevelString(LogLevel level);
    static const char* getLevelColor(LogLevel level);
    static void resetColor();

    template<typename T>
    static void log(LogLevel level, const T& message) {
        if (level < current_level_) return;
        std::ostringstream oss;
        oss << message;
        printLog(level, oss.str());
    }

    static void printLog(LogLevel level, const std::string& message);
};

}
