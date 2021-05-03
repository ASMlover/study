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

namespace tadpole::colorful {

#define set_colorful    set_foreground_colorful
#define SET_COLORFUL(c) set_colorful(stream, Colorful::c)
#define SET_FG(c)       set_foreground_colorful(stream, Colorful::FG_##c)
#define SET_BG(c)       set_background_colorful(stream, Colorful::BG_##c)

enum class Colorful : u8_t {
  RESET,

  FG_BLACK,
  FG_RED,
  FG_GREEN,
  FG_YELLOW,
  FG_BLUE,
  FG_MAGENTA,
  FG_CYAN,
  FG_WHITE,
  FG_GRAY,
  FG_LIGHTRED,
  FG_LIGHTGREEN,
  FG_LIGHTYELLOW,
  FG_LIGHTBLUE,
  FG_LIGHTMAGENTA,
  FG_LIGHTCYAN,
  FG_LIGHTWHITE,

  BG_BLACK,
  BG_RED,
  BG_GREEN,
  BG_YELLOW,
  BG_BLUE,
  BG_MAGENTA,
  BG_CYAN,
  BG_WHITE,
  BG_GRAY,
  BG_LIGHTRED,
  BG_LIGHTGREEN,
  BG_LIGHTYELLOW,
  BG_LIGHTBLUE,
  BG_LIGHTMAGENTA,
  BG_LIGHTCYAN,
  BG_LIGHTWHITE,
};

std::ostream& set_foreground_colorful(std::ostream& stream, Colorful c) noexcept;
std::ostream& set_background_colorful(std::ostream& stream, Colorful c) noexcept;

inline std::ostream& reset(std::ostream& stream) noexcept { return SET_COLORFUL(RESET); }
namespace fg {
  inline std::ostream& black(std::ostream& stream) noexcept { return SET_FG(BLACK); }
  inline std::ostream& red(std::ostream& stream) noexcept { return SET_FG(RED); }
  inline std::ostream& green(std::ostream& stream) noexcept { return SET_FG(GREEN); }
  inline std::ostream& yellow(std::ostream& stream) noexcept { return SET_FG(YELLOW); }
  inline std::ostream& blue(std::ostream& stream) noexcept { return SET_FG(BLUE); }
  inline std::ostream& magenta(std::ostream& stream) noexcept { return SET_FG(MAGENTA); }
  inline std::ostream& cyan(std::ostream& stream) noexcept { return SET_FG(CYAN); }
  inline std::ostream& white(std::ostream& stream) noexcept { return SET_FG(WHITE); }
  inline std::ostream& gray(std::ostream& stream) noexcept { return SET_FG(GRAY); }
  inline std::ostream& lightred(std::ostream& stream) noexcept { return SET_FG(LIGHTRED); }
  inline std::ostream& lightgreen(std::ostream& stream) noexcept { return SET_FG(LIGHTGREEN); }
  inline std::ostream& lightyellow(std::ostream& stream) noexcept { return SET_FG(LIGHTYELLOW); }
  inline std::ostream& lightblue(std::ostream& stream) noexcept { return SET_FG(LIGHTBLUE); }
  inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return SET_FG(LIGHTMAGENTA); }
  inline std::ostream& lightcyan(std::ostream& stream) noexcept { return SET_FG(LIGHTCYAN); }
  inline std::ostream& lightwhite(std::ostream& stream) noexcept { return SET_FG(LIGHTWHITE); }
}

namespace bg {
  inline std::ostream& black(std::ostream& stream) noexcept { return SET_BG(BLACK); }
  inline std::ostream& red(std::ostream& stream) noexcept { return SET_BG(RED); }
  inline std::ostream& green(std::ostream& stream) noexcept { return SET_BG(GREEN); }
  inline std::ostream& yellow(std::ostream& stream) noexcept { return SET_BG(YELLOW); }
  inline std::ostream& blue(std::ostream& stream) noexcept { return SET_BG(BLUE); }
  inline std::ostream& magenta(std::ostream& stream) noexcept { return SET_BG(MAGENTA); }
  inline std::ostream& cyan(std::ostream& stream) noexcept { return SET_BG(CYAN); }
  inline std::ostream& white(std::ostream& stream) noexcept { return SET_BG(WHITE); }
  inline std::ostream& gray(std::ostream& stream) noexcept { return SET_BG(GRAY); }
  inline std::ostream& lightred(std::ostream& stream) noexcept { return SET_BG(LIGHTRED); }
  inline std::ostream& lightgreen(std::ostream& stream) noexcept { return SET_BG(LIGHTGREEN); }
  inline std::ostream& lightyellow(std::ostream& stream) noexcept { return SET_BG(LIGHTYELLOW); }
  inline std::ostream& lightblue(std::ostream& stream) noexcept { return SET_BG(LIGHTBLUE); }
  inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return SET_BG(LIGHTMAGENTA); }
  inline std::ostream& lightcyan(std::ostream& stream) noexcept { return SET_BG(LIGHTCYAN); }
  inline std::ostream& lightwhite(std::ostream& stream) noexcept { return SET_BG(LIGHTWHITE); }
}

}
