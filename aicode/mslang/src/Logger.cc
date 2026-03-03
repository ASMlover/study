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
#include "Logger.hh"

namespace ms {

void Logger::log_impl(LogLevel level, strv_t tag, strv_t message) noexcept {
  if (level < level_)
    return;

  cstr_t level_name{};
  ColorManip color = fg::white;

  switch (level) {
  case LogLevel::TRACE: level_name = "TRACE"; color = fg::gray;    break;
  case LogLevel::DEBUG: level_name = "DEBUG"; color = fg::cyan;    break;
  case LogLevel::INFO:  level_name = "INFO";  color = fg::green;   break;
  case LogLevel::WARN:  level_name = "WARN";  color = fg::yellow;  break;
  case LogLevel::ERROR: level_name = "ERROR"; color = fg::red;     break;
  }

  std::cerr << color << "[" << level_name << "]" << reset_colorful
            << " [" << tag << "] " << message << std::endl;
}

void Logger::trace(strv_t tag, strv_t message) noexcept {
  if (level_ > LogLevel::TRACE) return;
  log_impl(LogLevel::TRACE, tag, message);
}

void Logger::debug(strv_t tag, strv_t message) noexcept {
  if (level_ > LogLevel::DEBUG) return;
  log_impl(LogLevel::DEBUG, tag, message);
}

void Logger::info(strv_t tag, strv_t message) noexcept {
  if (level_ > LogLevel::INFO) return;
  log_impl(LogLevel::INFO, tag, message);
}

void Logger::warn(strv_t tag, strv_t message) noexcept {
  if (level_ > LogLevel::WARN) return;
  log_impl(LogLevel::WARN, tag, message);
}

void Logger::error(strv_t tag, strv_t message) noexcept {
  if (level_ > LogLevel::ERROR) return;
  log_impl(LogLevel::ERROR, tag, message);
}

} // namespace ms
