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
#include "common.hh"
#if defined(TADPOLE_GNUC)
# include <unistd.h>
#else
# include <io.h>
# include <windows.h>
#endif
#include "colorful.hh"

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
inline const char* get_colorful(Colorful c) noexcept {
  switch (c) {
  case Colorful::RESET: return "\033[00m";
  case Colorful::FG_BLACK: return "\033[30m";
  case Colorful::FG_RED: return "\033[31m";
  case Colorful::FG_GREEN: return "\033[32m";
  case Colorful::FG_YELLOW: return "\033[33m";
  case Colorful::FG_BLUE: return "\033[34m";
  case Colorful::FG_MAGENTA: return "\033[35m";
  case Colorful::FG_CYAN: return "\033[36m";
  case Colorful::FG_WHITE: return "\033[37m";
  case Colorful::FG_GRAY: return "\033[90m";
  case Colorful::FG_LIGHTRED: return "\033[91m";
  case Colorful::FG_LIGHTGREEN: return "\033[92m";
  case Colorful::FG_LIGHTYELLOW: return "\033[93m";
  case Colorful::FG_LIGHTBLUE: return "\033[94m";
  case Colorful::FG_LIGHTMAGENTA: return "\033[95m";
  case Colorful::FG_LIGHTCYAN: return "\033[96m";
  case Colorful::FG_LIGHTWHITE: return "\033[97m";
  case Colorful::BG_BLACK: return "\033[40m";
  case Colorful::BG_RED: return "\033[41m";
  case Colorful::BG_GREEN: return "\033[42m";
  case Colorful::BG_YELLOW: return "\033[43m";
  case Colorful::BG_BLUE: return "\033[44m";
  case Colorful::BG_MAGENTA: return "\033[45m";
  case Colorful::BG_CYAN: return "\033[46m";
  case Colorful::BG_WHITE: return "\033[47m";
  case Colorful::BG_GRAY: return "\033[100m";
  case Colorful::BG_LIGHTRED: return "\033[101m";
  case Colorful::BG_LIGHTGREEN: return "\033[102m";
  case Colorful::BG_LIGHTYELLOW: return "\033[103m";
  case Colorful::BG_LIGHTBLUE: return "\033[104m";
  case Colorful::BG_LIGHTMAGENTA: return "\033[105m";
  case Colorful::BG_LIGHTCYAN: return "\033[106m";
  case Colorful::BG_LIGHTWHITE: return "\033[107m";
  }
  return "\033[00m";
}
#else
inline int get_colorful(Colorful c) noexcept {
  switch (c) {
  case Colorful::RESET: return -1;
  case Colorful::FG_BLACK: return FOREGROUND_RED & FOREGROUND_GREEN & FOREGROUND_BLUE;
  case Colorful::FG_RED: return FOREGROUND_RED;
  case Colorful::FG_GREEN: return FOREGROUND_GREEN;
  case Colorful::FG_YELLOW: return FOREGROUND_RED | FOREGROUND_GREEN;
  case Colorful::FG_BLUE: return FOREGROUND_BLUE;
  case Colorful::FG_MAGENTA: return FOREGROUND_RED | FOREGROUND_BLUE;
  case Colorful::FG_CYAN: return FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Colorful::FG_WHITE: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Colorful::FG_GRAY: return FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTRED: return FOREGROUND_RED | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTGREEN: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTYELLOW: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTBLUE: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTMAGENTA: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTCYAN: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Colorful::FG_LIGHTWHITE: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Colorful::BG_BLACK: return BACKGROUND_RED & BACKGROUND_GREEN & BACKGROUND_BLUE;
  case Colorful::BG_RED: return BACKGROUND_RED;
  case Colorful::BG_GREEN: return BACKGROUND_GREEN;
  case Colorful::BG_YELLOW: return BACKGROUND_RED | BACKGROUND_GREEN;
  case Colorful::BG_BLUE: return BACKGROUND_BLUE;
  case Colorful::BG_MAGENTA: return BACKGROUND_RED | BACKGROUND_BLUE;
  case Colorful::BG_CYAN: return BACKGROUND_GREEN | BACKGROUND_BLUE;
  case Colorful::BG_WHITE: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
  case Colorful::BG_GRAY: return BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTRED: return BACKGROUND_RED | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTGREEN: return BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTYELLOW: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTBLUE: return BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTMAGENTA: return BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTCYAN: return BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Colorful::BG_LIGHTWHITE: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  }
  return -1;
}

inline std::ostream& set_colorful(
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

std::ostream& set_foreground_colorful(std::ostream& stream, Colorful c) noexcept {
#if defined(TADPOLE_GNUC)
  return stream << get_colorful(c);
#else
  return set_colorful(stream, get_colorful(c));
#endif
}

std::ostream& set_background_colorful(std::ostream& stream, Colorful c) noexcept {
#if defined(TADPOLE_GNUC)
  return stream << get_colorful(c);
#else
  return set_colorful(stream, -1, get_colorful(c));
#endif
}

}
