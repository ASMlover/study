// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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

#include "common.hh"
#if defined(TADPOLE_GNUC)
# include <unistd.h>
#else
# include <io.h>
# include <windows.h>
#endif

namespace tadpole::colorful {

inline FILE* get_standard_stream(const std::ostream& stream) noexcept {
  if (&stream == &std::cout)
    return stdout;
  else if ((&stream == &std::cerr) || (&stream == &std::clog))
    return stderr;
  return nullptr;
}

inline bool is_atty(std::ostream& stream) noexcept {
  FILE* std_stream = get_standard_stream(stream);
  if (!std_stream)
    return false;

#if defined(TADPOLE_GNUC)
  return ::isatty(::fileno(std_stream));
#else
  return ::_isatty(::_fileno(std_stream));
#endif
}

#if defined(TADPOLE_GNUC)
# define COLORFUL_RESET               "\033[00m"
# define COLORFUL_FOREGROUND_RED      "\033[31m"
# define COLORFUL_FOREGROUND_GREEN    "\033[32m"
# define COLORFUL_FOREGROUND_YELLOW   "\033[33m"
# define COLORFUL_FOREGROUND_BLUE     "\033[34m"
# define COLORFUL_FOREGROUND_MAGENTA  "\033[35m"
# define COLORFUL_FOREGROUND_CYAN     "\033[36m"
# define COLORFUL_FOREGROUND_WHITE    "\033[37m"
# define COLORFUL_FOREGROUND_GRAY     "\033[90m"

# define SET_COLOR(c)                 (stream) << (c)
#else
# define COLORFUL_RESET               -1
# define COLORFUL_FOREGROUND_RED      FOREGOUND_RED
# define COLORFUL_FOREGROUND_GREEN    FOREGROUND_GREEN
# define COLORFUL_FOREGROUND_YELLOW   FOREGOUND_RED | FOREGROUND_GREEN
# define COLORFUL_FOREGROUND_BLUE     FOREGROUND_BLUE
# define COLORFUL_FOREGROUND_MAGENTA  FOREGOUND_RED | FOREGROUND_BLUE
# define COLORFUL_FOREGROUND_CYAN     FOREGROUND_GREEN | FOREGROUND_BLUE
# define COLORFUL_FOREGROUND_WHITE    FOREGOUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
# define COLORFUL_FOREGROUND_GRAY     FOREGROUND_INTENSITY

# define SET_COLOR(c)                 set_color((stream), (c))

inline std::ostream& set_color(
    std::ostream& stream, int foreground = -1, int background = -1) noexcept {
  static WORD wDefaultAttributes = 0;

  if (!is_atty(stream))
    return stream;

  HANDLE hTerminal = INVALID_HANDLE_VALUE;
  if (&stream == &std::cout)
    hTerminal = ::GetStdHandle(STD_OUTPUT_HANDLE);
  else if (&stream == &std::cerr)
    hTerminal = ::GetStdHandle(STD_ERROR_HANDLE);

  if (!wDefaultAttributes) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!::GetConsoleScreenBufferInfo(hTerminal, &info))
      return stream;
    wDefaultAttributes = info.wAttributes;
  }
  if (foreground == -1 && background == -1) {
    ::SetConsoleTextAttribute(hTerminal, wDefaultAttributes);
    return stream;
  }

  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!::GetConsoleScreenBufferInfo(hTerminal, &info))
    return stream;

  if (foreground != -1) {
    info.wAttributes &= ~(info.wAttributes & 0x0F);
    info.wAttributes |= as_type<WORD>(foreground);
  }
  if (background != -1) {
    info.wAttributes &= ~(info.wAttributes & 0xF0);
    info.wAttributes |= as_type<WORD>(background);
  }

  ::SetConsoleTextAttribute(hTerminal, info.wAttributes);
  return stream;
}
#endif

inline std::ostream& reset(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_RESET); }
inline std::ostream& red(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_RED); }
inline std::ostream& green(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_GREEN); }
inline std::ostream& yellow(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_YELLOW); }
inline std::ostream& blue(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_BLUE); }
inline std::ostream& magenta(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_MAGENTA); }
inline std::ostream& cyan(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_CYAN); }
inline std::ostream& white(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_WHITE); }
inline std::ostream& gray(std::ostream& stream) noexcept { return SET_COLOR(COLORFUL_FOREGROUND_WHITE); }

}
