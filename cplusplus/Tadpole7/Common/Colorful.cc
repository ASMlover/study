// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Common/Macros.hh>
#if defined(TADPOLE_MSVC)
# include <io.h>
# include <Windows.h>
#else
# include <unistd.h>
#endif
#include <Common/Colorful.hh>

namespace Tadpole::Common::Colorful {

inline FILE* get_standard_stream(const std::ostream& stream) noexcept {
  if (&stream == &std::cout)
    return stdout;
  else if (&stream == &std::cerr || &stream == &std::clog)
    return stderr;
  return nullptr;
}

inline bool is_atty(std::ostream& stream) noexcept {
  FILE* std_stream = get_standard_stream(stream);
  if (!std_stream)
    return false;

#if defined(TADPOLE_MSVC)
  return ::_isatty(::_fileno(std_stream));
#else
  return ::isatty(::fileno(std_stream));
#endif
}

#if defined(TADPOLE_MSVC)
inline int get_colorful(Color c) noexcept {
  switch (c) {
  case Color::kReset: return -1;
  case Color::kForegroundBlack: return FOREGROUND_RED & FOREGROUND_GREEN & FOREGROUND_BLUE;
  case Color::kForegroundRed: return FOREGROUND_RED;
  case Color::kForegroundGreen: return FOREGROUND_GREEN;
  case Color::kForegroundYellow: return FOREGROUND_RED | FOREGROUND_GREEN;
  case Color::kForegroundBlue: return FOREGROUND_BLUE;
  case Color::kForegroundMagenta: return FOREGROUND_RED | FOREGROUND_BLUE;
  case Color::kForegroundCyan: return FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Color::kForegroundWhite: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Color::kForegroundGray: return FOREGROUND_INTENSITY;
  case Color::kForegroundLightRed: return FOREGROUND_INTENSITY | FOREGROUND_RED;
  case Color::kForegroundLightGreen: return FOREGROUND_INTENSITY | FOREGROUND_GREEN;
  case Color::kForegroundLightYellow: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
  case Color::kForegroundLightBlue: return FOREGROUND_INTENSITY | FOREGROUND_BLUE;
  case Color::kForegroundLightMagenta: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;
  case Color::kForegroundLightCyan: return FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Color::kForegroundLightWhite: return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  case Color::kBackgroundBlack: return BACKGROUND_RED & BACKGROUND_GREEN & BACKGROUND_BLUE;
  case Color::kBackgroundRed: return BACKGROUND_RED;
  case Color::kBackgroundGreen: return BACKGROUND_GREEN;
  case Color::kBackgroundYellow: return BACKGROUND_RED | BACKGROUND_GREEN;
  case Color::kBackgroundBlue: return BACKGROUND_BLUE;
  case Color::kBackgroundMagenta: return BACKGROUND_RED | BACKGROUND_BLUE;
  case Color::kBackgroundCyan: return BACKGROUND_GREEN | BACKGROUND_BLUE;
  case Color::kBackgroundWhite: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
  case Color::kBackgroundGray: return BACKGROUND_INTENSITY;
  case Color::kBackgroundLightRed: return BACKGROUND_INTENSITY | BACKGROUND_RED;
  case Color::kBackgroundLightGreen: return BACKGROUND_INTENSITY | BACKGROUND_GREEN;
  case Color::kBackgroundLightYellow: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN;
  case Color::kBackgroundLightBlue: return BACKGROUND_INTENSITY | BACKGROUND_BLUE;
  case Color::kBackgroundLightMagenta: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE;
  case Color::kBackgroundLightCyan: return BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE;
  case Color::kBackgroundLightWhite: return BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
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

  WORD wAttributes;
  if (foreground == -1 && background == -1) {
    wAttributes = wDefaultAttributes;
  }
  else {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!::GetConsoleScreenBufferInfo(hTerminal, &info))
      return stream;

    wAttributes = info.wAttributes;
    if (foreground != -1) {
      wAttributes &= ~(info.wAttributes & 0x0F);
      wAttributes |= as_type<WORD>(foreground);
    }
    if (background != -1) {
      wAttributes &= ~(info.wAttributes & 0xF0);
      wAttributes |= as_type<WORD>(background);
    }
  }

  ::SetConsoleTextAttribute(hTerminal, wAttributes);
  return stream;
}
#else
inline const char* get_colorful(Color c) noexcept {
  switch (c) {
  case Color::kReset: return "\033[00m";
  case Color::kForegroundBlack: return "\033[30m";
  case Color::kForegroundRed: return "\033[31m";
  case Color::kForegroundGreen: return "\033[32m";
  case Color::kForegroundYellow: return "\033[33m";
  case Color::kForegroundBlue: return "\033[34m";
  case Color::kForegroundMagenta: return "\033[35m";
  case Color::kForegroundCyan: return "\033[36m";
  case Color::kForegroundWhite: return "\033[37m";
  case Color::kForegroundGray: return "\033[90m";
  case Color::kForegroundLightRed: return "\033[91m";
  case Color::kForegroundLightGreen: return "\033[92m";
  case Color::kForegroundLightYellow: return "\033[93m";
  case Color::kForegroundLightBlue: return "\033[94m";
  case Color::kForegroundLightMagenta: return "\033[95m";
  case Color::kForegroundLightCyan: return "\033[96m";
  case Color::kForegroundLightWhite: return "\033[97m";
  case Color::kBackgroundBlack: return "\033[40m";
  case Color::kBackgroundRed: return "\033[41m";
  case Color::kBackgroundGreen: return "\033[42m";
  case Color::kBackgroundYellow: return "\033[43m";
  case Color::kBackgroundBlue: return "\033[44m";
  case Color::kBackgroundMagenta: return "\033[45m";
  case Color::kBackgroundCyan: return "\033[46m";
  case Color::kBackgroundWhite: return "\033[47m;
  case Color::kBackgroundGray: return "\033[100m";
  case Color::kBackgroundLightRed: return "\033[101m";
  case Color::kBackgroundLightGreen: return "\033[102m";
  case Color::kBackgroundLightYellow: return "\033[103m";
  case Color::kBackgroundLightBlue: return "\033[104m";
  case Color::kBackgroundLightMagenta: return "\033[105m";
  case Color::kBackgroundLightCyan: return "\033[106m";
  case Color::kBackgroundLightWhite: return "\033[107m";
  }
  return "\033[00m";
}
#endif

std::ostream& set_colorful(std::ostream& stream, Color color) noexcept {
#if defined(TADPOLE_MSVC)
  return set_colorful(stream, get_colorful(color));
#else
  return stream << get_colorful(color);
#endif
}

std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept {
  return set_colorful(stream, color);
}

std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept {
#if defined(TADPOLE_MSVC)
  return set_colorful(stream, -1, get_colorful(color));
#else
  return stream << get_colorful(color)
#endif
}

}
