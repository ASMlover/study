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

#include <Common/Common.hh>

namespace Tadpole::Common::Colorful {

enum class Color : u8_t {
  kReset,

  kForegroundBlack,
  kForegroundRed,
  kForegroundGreen,
  kForegroundYellow,
  kForegroundBlue,
  kForegroundMagenta,
  kForegroundCyan,
  kForegroundWhite,
  kForegroundGray,
  kForegroundLightRed,
  kForegroundLightGreen,
  kForegroundLightYellow,
  kForegroundLightBlue,
  kForegroundLightMagenta,
  kForegroundLightCyan,
  kForegroundLightWhite,

  kBackgroundBlack,
  kBackgroundRed,
  kBackgroundGreen,
  kBackgroundYellow,
  kBackgroundBlue,
  kBackgroundMagenta,
  kBackgroundCyan,
  kBackgroundWhite,
  kBackgroundGray,
  kBackgroundLightRed,
  kBackgroundLightGreen,
  kBackgroundLightYellow,
  kBackgroundLightBlue,
  kBackgroundLightMagenta,
  kBackgroundLightCyan,
  kBackgroundLightWhite,
};

std::ostream& set_colorful(std::ostream& stream, Color color) noexcept;
std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept;
std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept;

inline std::ostream& reset(std::ostream& stream) noexcept { return set_colorful(stream, Color::kReset); }

namespace fg {
#define FGCOLOR(c) set_foreground_colorful(stream, Color::kForeground##c)

  inline std::ostream& black(std::ostream& stream) noexcept { return FGCOLOR(Black); }
  inline std::ostream& red(std::ostream& stream) noexcept { return FGCOLOR(Red); }
  inline std::ostream& green(std::ostream& stream) noexcept { return FGCOLOR(Green); }
  inline std::ostream& yellow(std::ostream& stream) noexcept { return FGCOLOR(Yellow); }
  inline std::ostream& blue(std::ostream& stream) noexcept { return FGCOLOR(Blue); }
  inline std::ostream& magenta(std::ostream& stream) noexcept { return FGCOLOR(Magenta); }
  inline std::ostream& cyan(std::ostream& stream) noexcept { return FGCOLOR(Cyan); }
  inline std::ostream& white(std::ostream& stream) noexcept { return FGCOLOR(White); }
  inline std::ostream& gray(std::ostream& stream) noexcept { return FGCOLOR(Gray); }
  inline std::ostream& lightred(std::ostream& stream) noexcept { return FGCOLOR(LightRed); }
  inline std::ostream& lightgreen(std::ostream& stream) noexcept { return FGCOLOR(LightGreen); }
  inline std::ostream& lightyellow(std::ostream& stream) noexcept { return FGCOLOR(LightYellow); }
  inline std::ostream& lightblue(std::ostream& stream) noexcept { return FGCOLOR(LightBlue); }
  inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return FGCOLOR(LightMagenta); }
  inline std::ostream& lightcyan(std::ostream& stream) noexcept { return FGCOLOR(LightCyan); }
  inline std::ostream& lightwhite(std::ostream& stream) noexcept { return FGCOLOR(LightWhite); }

#undef FGCOLOR
}

namespace bg {
#define BGCOLOR(c) set_background_colorful(stream, Color::kBackground##c)

  inline std::ostream& black(std::ostream& stream) noexcept { return BGCOLOR(Black); }
  inline std::ostream& red(std::ostream& stream) noexcept { return BGCOLOR(Red); }
  inline std::ostream& green(std::ostream& stream) noexcept { return BGCOLOR(Green); }
  inline std::ostream& yellow(std::ostream& stream) noexcept { return BGCOLOR(Yellow); }
  inline std::ostream& blue(std::ostream& stream) noexcept { return BGCOLOR(Blue); }
  inline std::ostream& magenta(std::ostream& stream) noexcept { return BGCOLOR(Magenta); }
  inline std::ostream& cyan(std::ostream& stream) noexcept { return BGCOLOR(Cyan); }
  inline std::ostream& white(std::ostream& stream) noexcept { return BGCOLOR(White); }
  inline std::ostream& gray(std::ostream& stream) noexcept { return BGCOLOR(Gray); }
  inline std::ostream& lightred(std::ostream& stream) noexcept { return BGCOLOR(LightRed); }
  inline std::ostream& lightgreen(std::ostream& stream) noexcept { return BGCOLOR(LightGreen); }
  inline std::ostream& lightyellow(std::ostream& stream) noexcept { return BGCOLOR(LightYellow); }
  inline std::ostream& lightblue(std::ostream& stream) noexcept { return BGCOLOR(LightBlue); }
  inline std::ostream& lightmagenta(std::ostream& stream) noexcept { return BGCOLOR(LightMagenta); }
  inline std::ostream& lightcyan(std::ostream& stream) noexcept { return BGCOLOR(LightCyan); }
  inline std::ostream& lightwhite(std::ostream& stream) noexcept { return BGCOLOR(LightWhite); }

#undef BGCOLOR
}

}
