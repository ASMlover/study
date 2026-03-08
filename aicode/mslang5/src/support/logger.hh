#pragma once

#include <iosfwd>
#include <mutex>
#include <string>
#include <string_view>

namespace ms {

enum class LogLevel {
  kTrace = 0,
  kDebug,
  kInfo,
  kWarn,
  kError,
  kFatal,
};

class Logger {
 public:
  static Logger& Instance();

  void SetMinLevel(LogLevel level);
  LogLevel MinLevel() const;

  void SetColorEnabled(bool enabled);
  bool ColorEnabled() const;

  void SetSink(std::ostream& sink);

  bool ShouldLog(LogLevel level) const;
  void Log(LogLevel level, std::string_view message);

 private:
  Logger();

  std::string_view LevelTag(LogLevel level) const;
  std::string_view LevelColor(LogLevel level) const;
  void TryEnableAnsiOnWindows();

  mutable std::mutex mutex_;
  LogLevel min_level_;
  bool color_enabled_;
  std::ostream* sink_;
};

}  // namespace ms

