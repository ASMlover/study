// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <iostream>
#include <format>
#include "Common.hh"
#include "Colorful.hh"

namespace ms {

enum class LogLevel : int {
  TRACE = 0,
  DEBUG,
  INFO,
  WARN,
  ERROR,
};

class Logger final : public Singleton<Logger> {
  LogLevel level_{LogLevel::INFO};

  void log_impl(LogLevel level, strv_t tag, strv_t message) noexcept;
public:
  void set_level(LogLevel level) noexcept { level_ = level; }
  LogLevel level() const noexcept { return level_; }

  void trace(strv_t tag, strv_t message) noexcept;
  void debug(strv_t tag, strv_t message) noexcept;
  void info(strv_t tag, strv_t message) noexcept;
  void warn(strv_t tag, strv_t message) noexcept;
  void error(strv_t tag, strv_t message) noexcept;

  template <typename... Args>
  void trace(strv_t tag, std::format_string<Args...> fmt, Args&&... args) noexcept {
    if (level_ > LogLevel::TRACE) return;
    log_impl(LogLevel::TRACE, tag, std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  void debug(strv_t tag, std::format_string<Args...> fmt, Args&&... args) noexcept {
    if (level_ > LogLevel::DEBUG) return;
    log_impl(LogLevel::DEBUG, tag, std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  void info(strv_t tag, std::format_string<Args...> fmt, Args&&... args) noexcept {
    if (level_ > LogLevel::INFO) return;
    log_impl(LogLevel::INFO, tag, std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  void warn(strv_t tag, std::format_string<Args...> fmt, Args&&... args) noexcept {
    if (level_ > LogLevel::WARN) return;
    log_impl(LogLevel::WARN, tag, std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  void error(strv_t tag, std::format_string<Args...> fmt, Args&&... args) noexcept {
    if (level_ > LogLevel::ERROR) return;
    log_impl(LogLevel::ERROR, tag, std::format(fmt, std::forward<Args>(args)...));
  }
};

} // namespace ms
