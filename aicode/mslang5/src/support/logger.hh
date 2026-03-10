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
  static Logger& instance();

  void set_min_level(LogLevel level);
  LogLevel min_level() const;

  void set_color_enabled(bool enabled);
  bool color_enabled() const;

  void set_sink(std::ostream& sink);

  bool should_log(LogLevel level) const;
  void log(LogLevel level, std::string_view message);

 private:
  Logger();

  std::string_view level_tag(LogLevel level) const;
  std::string_view level_color(LogLevel level) const;
  void try_enable_ansi_on_windows();

  mutable std::mutex mutex_;
  LogLevel min_level_;
  bool color_enabled_;
  std::ostream* sink_;
};

}  // namespace ms
