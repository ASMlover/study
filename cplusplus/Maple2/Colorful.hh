// Copyright (c) 2025 ASMlover. All rights reserved.
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

#include "Types.hh"

namespace ms {

enum class Color : u8_t {
  kREST,

  kFOREGROUND_BLACK,
  kFOREGROUND_RED,
  kFOREGROUND_GREEN,
  kFOREGROUND_YELLOW,
  kFOREGROUND_BLUE,
  kFOREGROUND_MAGENTA,
  kFOREGROUND_CYAN,
  kFOREGROUND_WHITE,
  kFOREGROUND_GRAY,
  kFOREGROUND_LIGHTRED,
  kFOREGROUND_LIGHTGREEN,
  kFOREGROUND_LIGHTYELLOW,
  kFOREGROUND_LIGHTBLUE,
  kFOREGROUND_LIGHTMAGENTA,
  kFOREGROUND_LIGHTCYAN,
  kFOREGROUND_LIGHTWHITE,

  kBACKGROUND_BLACK,
  kBACKGROUND_RED,
  kBACKGROUND_GREEN,
  kBACKGROUND_YELLOW,
  kBACKGROUND_BLUE,
  kBACKGROUND_MAGENTA,
  kBACKGROUND_CYAN,
  kBACKGROUND_WHITE,
  kBACKGROUND_GRAY,
  kBACKGROUND_LIGHTRED,
  kBACKGROUND_LIGHTGREEN,
  kBACKGROUND_LIGHTYELLOW,
  kBACKGROUND_LIGHTBLUE,
  kBACKGROUND_LIGHTMAGENTA,
  kBACKGROUND_LIGHTCYAN,
  kBACKGROUND_LIGHTWHITE,
};

std::ostream& set_colorful(std::ostream& stream, Color color) noexcept;
std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept;
std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept;

inline std::ostream& reset_colorful(std::ostream& stream) noexcept {
  return set_colorful(stream, Color::kREST);
}

#define FGCOLOR(Name, C) \
  inline std::ostream& Name(std::ostream& stream) noexcept {\
    return set_foreground_colorful(stream, Color::kFOREGROUND_##C);\
  }
#define BGCOLOR(Name, C) \
  inline std::ostream& Name(std::ostream& stream) noexcept {\
    return set_background_colorful(stream, Color::kBACKGROUND_##C);\
  }

namespace fg {

  FGCOLOR(black,        BLACK)
  FGCOLOR(red,          RED)
  FGCOLOR(green,        GREEN)
  FGCOLOR(yellow,       YELLOW)
  FGCOLOR(blue,         BLUE)
  FGCOLOR(magenta,      MAGENTA)
  FGCOLOR(cyan,         CYAN)
  FGCOLOR(white,        WHITE)
  FGCOLOR(gray,         GRAY)
  FGCOLOR(lightred,     LIGHTRED)
  FGCOLOR(lightgreen,   LIGHTGREEN)
  FGCOLOR(lightyellow,  LIGHTYELLOW)
  FGCOLOR(lightblue,    LIGHTBLUE)
  FGCOLOR(lightmagenta, LIGHTMAGENTA)
  FGCOLOR(lightcyan,    LIGHTCYAN)
  FGCOLOR(lightwhite,   LIGHTWHITE)

}

namespace bg {

  BGCOLOR(black,        BLACK)
  BGCOLOR(red,          RED)
  BGCOLOR(green,        GREEN)
  BGCOLOR(yellow,       YELLOW)
  BGCOLOR(blue,         BLUE)
  BGCOLOR(magenta,      MAGENTA)
  BGCOLOR(cyan,         CYAN)
  BGCOLOR(white,        WHITE)
  BGCOLOR(gray,         GRAY)
  BGCOLOR(lightred,     LIGHTRED)
  BGCOLOR(lightgreen,   LIGHTGREEN)
  BGCOLOR(lightyellow,  LIGHTYELLOW)
  BGCOLOR(lightblue,    LIGHTBLUE)
  BGCOLOR(lightmagenta, LIGHTMAGENTA)
  BGCOLOR(lightcyan,    LIGHTCYAN)
  BGCOLOR(lightwhite,   LIGHTWHITE)

}

}
