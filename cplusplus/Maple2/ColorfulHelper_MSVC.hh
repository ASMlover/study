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

#include <io.h>
#include <Windows.h>
#include "ColorfulCommon.hh"
#include "Colorful.hh"

namespace ms::colorful {

inline bool is_atty(std::ostream& stream) noexcept {
  FILE* std_stream = get_standard_stream(stream);
  if (!std_stream)
    return false;

  return ::_isatty(::_fileno(std_stream));
}

inline int get_colorful(Color c) noexcept {
#define COLORMAP(c, v)    case Color::k##c: return v
#define FGCOLORMAP(c, v)  case Color::kFOREGROUND_##c: return v
#define BGCOLORMAP(c, v)  case Color::kBACKGROUND_##c: return v

  switch (c) {
  COLORMAP(RESET,           -1);
  FGCOLORMAP(BLACK,         FOREGROUND_RED & FOREGROUND_GREEN & FOREGROUND_BLUE);
  FGCOLORMAP(RED,           FOREGROUND_RED);
  FGCOLORMAP(GREEN,         FOREGROUND_GREEN);
  FGCOLORMAP(YELLOW,        FOREGROUND_RED | FOREGROUND_GREEN);
  FGCOLORMAP(BLUE,          FOREGROUND_BLUE);
  FGCOLORMAP(MAGENTA,       FOREGROUND_RED | FOREGROUND_BLUE);
  FGCOLORMAP(CYAN,          FOREGROUND_GREEN | FOREGROUND_BLUE);
  FGCOLORMAP(WHITE,         FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  FGCOLORMAP(GRAY,          FOREGROUND_INTENSITY);
  FGCOLORMAP(LIGHTRED,      FOREGROUND_INTENSITY | FOREGROUND_RED);
  FGCOLORMAP(LIGHTGREEN,    FOREGROUND_INTENSITY | FOREGROUND_GREEN);
  FGCOLORMAP(LIGHTYELLOW,   FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
  FGCOLORMAP(LIGHTBLUE,     FOREGROUND_INTENSITY | FOREGROUND_BLUE);
  FGCOLORMAP(LIGHTMAGENTA,  FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
  FGCOLORMAP(LIGHTCYAN,     FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
  FGCOLORMAP(LIGHTWHITE,    FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  BGCOLORMAP(BLACK,         BACKGROUND_RED & BACKGROUND_GREEN & BACKGROUND_BLUE);
  BGCOLORMAP(RED,           BACKGROUND_RED);
  BGCOLORMAP(GREEN,         BACKGROUND_GREEN);
  BGCOLORMAP(YELLOW,        BACKGROUND_RED | BACKGROUND_GREEN);
  BGCOLORMAP(BLUE,          BACKGROUND_BLUE);
  BGCOLORMAP(MAGENTA,       BACKGROUND_RED | BACKGROUND_BLUE);
  BGCOLORMAP(CYAN,          BACKGROUND_GREEN | BACKGROUND_BLUE);
  BGCOLORMAP(WHITE,         BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
  BGCOLORMAP(GRAY,          BACKGROUND_INTENSITY);
  BGCOLORMAP(LIGHTRED,      BACKGROUND_INTENSITY | BACKGROUND_RED);
  BGCOLORMAP(LIGHTGREEN,    BACKGROUND_INTENSITY | BACKGROUND_GREEN);
  BGCOLORMAP(LIGHTYELLOW,   BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
  BGCOLORMAP(LIGHTBLUE,     BACKGROUND_INTENSITY | BACKGROUND_BLUE);
  BGCOLORMAP(LIGHTMAGENTA,  BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE);
  BGCOLORMAP(LIGHTCYAN,     BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE);
  BGCOLORMAP(LIGHTWHITE,    BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
  default: break;
  }

#undef BGCOLORMAP
#undef FGCOLORMAP
#undef COLORMAP
  return -1;
}

inline std::ostream& set_colorful_impl(std::ostream& stream, int foreground = -1, int background = -1) noexcept {
  // TODO:
  return stream;
}

inline std::ostream& set_colorful(std::ostream& stream, Color color) noexcept {
  return set_colorful_impl(stream, get_colorful(color));
}

inline std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept {
  return set_colorful_impl(stream, get_colorful(color));
}

inline std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept {
  return set_colorful_impl(stream, -1, get_colorful(color));
}

}
