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
#pragma once

#include "common.hh"

namespace tadpole::colorful {

enum class Colorful : u8_t {
  COLORFUL_RESET,

  COLORFUL_FOREGROUND_BLACK,
  COLORFUL_FOREGROUND_RED,
  COLORFUL_FOREGROUND_GREEN,
  COLORFUL_FOREGROUND_YELLOW,
  COLORFUL_FOREGROUND_BLUE,
  COLORFUL_FOREGROUND_MAGENTA,
  COLORFUL_FOREGROUND_CYAN,
  COLORFUL_FOREGROUND_WHITE,
  COLORFUL_FOREGROUND_GRAY,
  COLORFUL_FOREGROUND_LIGHTRED,
  COLORFUL_FOREGROUND_LIGHTGREEN,
  COLORFUL_FOREGROUND_LIGHTYELLOW,
  COLORFUL_FOREGROUND_LIGHTBLUE,
  COLORFUL_FOREGROUND_LIGHTMAGENTA,
  COLORFUL_FOREGROUND_LIGHTCYAN,
  COLORFUL_FOREGROUND_LIGHTWHITE,

  COLORFUL_BACKGROUND_BLACK,
  COLORFUL_BACKGROUND_RED,
  COLORFUL_BACKGROUND_GREEN,
  COLORFUL_BACKGROUND_YELLOW,
  COLORFUL_BACKGROUND_BLUE,
  COLORFUL_BACKGROUND_MAGENTA,
  COLORFUL_BACKGROUND_CYAN,
  COLORFUL_BACKGROUND_WHITE,
  COLORFUL_BACKGROUND_GRAY,
  COLORFUL_BACKGROUND_LIGHTRED,
  COLORFUL_BACKGROUND_LIGHTGREEN,
  COLORFUL_BACKGROUND_LIGHTYELLOW,
  COLORFUL_BACKGROUND_LIGHTBLUE,
  COLORFUL_BACKGROUND_LIGHTMAGENTA,
  COLORFUL_BACKGROUND_LIGHTCYAN,
  COLORFUL_BACKGROUND_LIGHTWHITE,
};

std::ostream& set_colorful(std::ostream& stream, Colorful color) noexcept;
std::ostream& set_foreground_colorful(std::ostream& stream, Colorful color) noexcept;
std::ostream& set_background_colorful(std::ostream& stream, Colorful color) noexcept;

inline std::ostream& reset(std::ostream& stream) noexcept { return set_colorful(stream, Colorful::COLORFUL_RESET); }

namespace fg {
#define SETCOLOR(c) set_foreground_colorful(stream, Colorful::COLORFUL_FOREGROUND_##c)

inline std::ostream& black(std::ostream& stream) noexcept { return SETCOLOR(BLACK); }
inline std::ostream& red(std::ostream& stream) noexcept { return SETCOLOR(RED); }
inline std::ostream& green(std::ostream& stream) noexcept { return SETCOLOR(GREEN); }
inline std::ostream& yellow(std::ostream& stream) noexcept { return SETCOLOR(YELLOW); }
inline std::ostream& blue(std::ostream& stream) noexcept { return SETCOLOR(BLUE); }
inline std::ostream& magenta(std::ostream& stream) noexcept { return SETCOLOR(MAGENTA); }
inline std::ostream& cyan(std::ostream& stream) noexcept { return SETCOLOR(CYAN); }
inline std::ostream& white(std::ostream& stream) noexcept { return SETCOLOR(WHITE); }
inline std::ostream& gray(std::ostream& stream) noexcept { return SETCOLOR(GRAY); }
inline std::ostream& lightred(std::ostream& stream) noexcept { return SETCOLOR(LIGHTRED); }
inline std::ostream& lightgreen(std::ostream& stream) noexcept { return SETCOLOR(LIGHTGREEN); }
inline std::ostream& lightyellow(std::ostream& stream) noexcept { return SETCOLOR(LIGHTYELLOW); }
inline std::ostream& lightblue(std::ostream& stream) noexcept { return SETCOLOR(LIGHTBLUE); }
inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return SETCOLOR(LIGHTMAGENTA); }
inline std::ostream& lightcyan(std::ostream& stream) noexcept { return SETCOLOR(LIGHTCYAN); }
inline std::ostream& lightwhite(std::ostream& stream) noexcept { return SETCOLOR(LIGHTWHITE); }

#undef SETCOLOR
}

namespace bg {
#define SETCOLOR(c) set_background_colorful(stream, Colorful::COLORFUL_BACKGROUND_##c)

inline std::ostream& black(std::ostream& stream) noexcept { return SETCOLOR(BLACK); }
inline std::ostream& red(std::ostream& stream) noexcept { return SETCOLOR(RED); }
inline std::ostream& green(std::ostream& stream) noexcept { return SETCOLOR(GREEN); }
inline std::ostream& yellow(std::ostream& stream) noexcept { return SETCOLOR(YELLOW); }
inline std::ostream& blue(std::ostream& stream) noexcept { return SETCOLOR(BLUE); }
inline std::ostream& magenta(std::ostream& stream) noexcept { return SETCOLOR(MAGENTA); }
inline std::ostream& cyan(std::ostream& stream) noexcept { return SETCOLOR(CYAN); }
inline std::ostream& white(std::ostream& stream) noexcept { return SETCOLOR(WHITE); }
inline std::ostream& gray(std::ostream& stream) noexcept { return SETCOLOR(GRAY); }
inline std::ostream& lightred(std::ostream& stream) noexcept { return SETCOLOR(LIGHTRED); }
inline std::ostream& lightgreen(std::ostream& stream) noexcept { return SETCOLOR(LIGHTGREEN); }
inline std::ostream& lightyellow(std::ostream& stream) noexcept { return SETCOLOR(LIGHTYELLOW); }
inline std::ostream& lightblue(std::ostream& stream) noexcept { return SETCOLOR(LIGHTBLUE); }
inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return SETCOLOR(LIGHTMAGENTA); }
inline std::ostream& lightcyan(std::ostream& stream) noexcept { return SETCOLOR(LIGHTCYAN); }
inline std::ostream& lightwhite(std::ostream& stream) noexcept { return SETCOLOR(LIGHTWHITE); }

#undef SETCOLOR
}

}
