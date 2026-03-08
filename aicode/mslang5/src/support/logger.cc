#include "support/logger.hh"

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace ms {

namespace {

constexpr std::string_view kColorReset = "\x1b[0m";

}  // namespace

Logger& Logger::Instance() {
  static Logger logger;
  return logger;
}

Logger::Logger()
    : min_level_(LogLevel::kInfo), color_enabled_(true), sink_(&std::cout) {
  TryEnableAnsiOnWindows();
}

void Logger::SetMinLevel(const LogLevel level) {
  std::lock_guard<std::mutex> lock(mutex_);
  min_level_ = level;
}

LogLevel Logger::MinLevel() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return min_level_;
}

void Logger::SetColorEnabled(const bool enabled) {
  std::lock_guard<std::mutex> lock(mutex_);
  color_enabled_ = enabled;
}

bool Logger::ColorEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return color_enabled_;
}

void Logger::SetSink(std::ostream& sink) {
  std::lock_guard<std::mutex> lock(mutex_);
  sink_ = &sink;
}

bool Logger::ShouldLog(const LogLevel level) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return static_cast<int>(level) >= static_cast<int>(min_level_);
}

void Logger::Log(const LogLevel level, const std::string_view message) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (static_cast<int>(level) < static_cast<int>(min_level_)) {
    return;
  }

  if (color_enabled_) {
    (*sink_) << LevelColor(level);
  }
  (*sink_) << "[" << LevelTag(level) << "] " << message;
  if (color_enabled_) {
    (*sink_) << kColorReset;
  }
  (*sink_) << '\n';
}

std::string_view Logger::LevelTag(const LogLevel level) const {
  switch (level) {
    case LogLevel::kTrace:
      return "TRACE";
    case LogLevel::kDebug:
      return "DEBUG";
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarn:
      return "WARN";
    case LogLevel::kError:
      return "ERROR";
    case LogLevel::kFatal:
      return "FATAL";
  }
  return "UNKNOWN";
}

std::string_view Logger::LevelColor(const LogLevel level) const {
  switch (level) {
    case LogLevel::kTrace:
      return "\x1b[90m";
    case LogLevel::kDebug:
      return "\x1b[36m";
    case LogLevel::kInfo:
      return "\x1b[32m";
    case LogLevel::kWarn:
      return "\x1b[33m";
    case LogLevel::kError:
      return "\x1b[31m";
    case LogLevel::kFatal:
      return "\x1b[35m";
  }
  return "";
}

void Logger::TryEnableAnsiOnWindows() {
#if defined(_WIN32)
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  if (out == INVALID_HANDLE_VALUE) {
    return;
  }
  DWORD mode = 0;
  if (!GetConsoleMode(out, &mode)) {
    return;
  }
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(out, mode);
#endif
}

}  // namespace ms

