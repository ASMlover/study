// Copyright (c) 2021 ASMlover. All rights reserved.
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
#include <iostream>

#if defined(__GNUC__) || defined(__clang__)
# define USE_POSIX
# include <unistd.h>
#else
# include <Windows.h>
#endif

namespace internal {

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

#if defined(USE_POSIX)
  return ::isatty(fileno(std_stream));
#else
  return ::_isatty(_fileno(std_stream));
#endif
}

}

#if defined(USE_POSIX)
# define KRST       "\033[00m"
# define KRED       "\033[31m"
# define KGREEN     "\033[32m"
# define KYELLOW    "\033[33m"
# define KBLUE      "\033[34m"
# define KMAGENTA   "\033[35m"
# define KCYAN      "\033[36m"
# define kWHITE     "\033[37m"
# define KGRAY      "\033[90m"

# define FOREGROUND_RED(x)      KRED x KRST
# define FOREGROUND_GREEN(x)    KGREEN x KRST
# define FOREGROUND_YELLOW(x)   KYELLOW x KRST
# define FOREGROUND_BLUE(x)     KBLUE x KRST
# define FOREGROUND_MAGENTA(x)  KMAGENTA x KRST
# define FOREGROUND_CYAN(x)     KCYAN x KRST
# define FOREGROUND_WHITE(x)    kWHITE x KRST
# define FOREGROUND_GRAY(x)     KGRAY x KRST

inline std::ostream& reset(std::ostream& out) noexcept { return out << "\033[00m"; }
inline std::ostream& red(std::ostream& out) noexcept { return out << "\033[31m"; }
inline std::ostream& green(std::ostream& out) noexcept { return out << "\033[32m"; }
inline std::ostream& yellow(std::ostream& out) noexcept { return out << "\033[33m"; }
inline std::ostream& blue(std::ostream& out) noexcept { return out << "\033[34m"; }
inline std::ostream& magenta(std::ostream& out) noexcept { return out << "\033[35m"; }
inline std::ostream& cyan(std::ostream& out) noexcept { return out << "\033[36m"; }
inline std::ostream& white(std::ostream& out) noexcept { return out << "\033[37m"; }
inline std::ostream& gray(std::ostream& out) noexcept { return out << "\033[90m"; }
#else
namespace internal {

inline void win_change_attributes(std::ostream& stream, int foreground, int background = -1) {
  static WORD wDefaultAttributes = 0;

  if (!is_atty(stream))
    return;

  HANDLE hTerminal = INVALID_HANDLE_VALUE;
  if (&stream == &std::cout)
    hTerminal = ::GetStdHandle(STD_OUTPUT_HANDLE);
  else if (&stream == &std::cerr)
    hTerminal = ::GetStdHandle(STD_ERROR_HANDLE);

  if (!wDefaultAttributes) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!::GetConsoleScreenBufferInfo(hTerminal, &info))
      return;
    wDefaultAttributes = info.wAttributes;
  }
  if (foreground == -1 && background == -1) {
    ::SetConsoleTextAttribute(hTerminal, wDefaultAttributes);
    return;
  }

  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!::GetConsoleScreenBufferInfo(hTerminal, &info))
    return;

  if (foreground != -1) {
    info.wAttributes &= ~(info.wAttributes & 0x0f);
    info.wAttributes |= static_cast<WORD>(foreground);
  }
  if (background != -1) {
    info.wAttributes &= ~(info.wAttributes & 0xf0);
    info.wAttributes |= static_cast<WORD>(background);
  }

  ::SetConsoleTextAttribute(hTerminal, info.wAttributes);
}

inline std::ostream& reset(std::ostream& stream) noexcept {
  internal::win_change_attributes(stream, -1, -1);
  return stream;
}

inline std::ostream& red(std::ostream& stream) noexcept {
  internal::win_change_attributes(stream, FOREGROUND_RED);
  return stream;
}

inline std::ostream& green(std::ostream& stream) noexcept {
  internal::win_change_attributes(stream, FOREGROUND_GREEN);
  return stream;
}

}
#endif

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << FOREGROUND_RED("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_GREEN("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_YELLOW("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_BLUE("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_MAGENTA("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_CYAN("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_WHITE("Hello, world!!!") << std::endl;
  std::cout << FOREGROUND_GRAY("Hello, world!!!") << std::endl;

  std::cout << red << "Hello, colorful world !!!" << std::endl;
  std::cout << green << "Hello, colorful world !!!" << std::endl;
  std::cout << yellow << "Hello, colorful world !!!" << std::endl;
  std::cout << blue << "Hello, colorful world !!!" << std::endl;
  std::cout << magenta << "Hello, colorful world !!!" << std::endl;
  std::cout << cyan << "Hello, colorful world !!!" << std::endl;
  std::cout << white << "Hello, colorful world !!!" << std::endl;
  std::cout << gray << "Hello, colorful world !!!" << std::endl;
  std::cout << reset;

  return 0;
}
