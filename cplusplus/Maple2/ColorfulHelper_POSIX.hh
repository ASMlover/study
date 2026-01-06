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

#include <unistd.h>
#include "ColorfulCommon.hh"

namespace ms::colorful {

inline bool is_atty(std::ostream& stream) noexcept {
  FILE* std_stream = get_standard_stream(stream);
  if (!std_stream)
    return false;

  return ::isatty(::fileno(std_stream));
}

inline const char* get_colorful(Color c) noexcept {
#define COLORMAP(c, v)    case Color::k##c: return v
#define FGCOLORMAP(c, v)  case Color::kFOREGROUND_##c: return v
#define BGCOLORMAP(c, v)  case Color::kBACKGROUND_##c: return v

  switch (c) {
  COLORMAP(RESET,           "\033[00m");
  FGCOLORMAP(BLACK,         "\033[30m");
  FGCOLORMAP(RED,           "\033[31m");
  FGCOLORMAP(GREEN,         "\033[32m");
  FGCOLORMAP(YELLOW,        "\033[33m");
  FGCOLORMAP(BLUE,          "\033[34m");
  FGCOLORMAP(MAGENTA,       "\033[35m");
  FGCOLORMAP(CYAN,          "\033[36m");
  FGCOLORMAP(WHITE,         "\033[37m");
  FGCOLORMAP(GRAY,          "\033[90m");
  FGCOLORMAP(LIGHTRED,      "\033[91m");
  FGCOLORMAP(LIGHTGREEN,    "\033[92m");
  FGCOLORMAP(LIGHTYELLOW,   "\033[93m");
  FGCOLORMAP(LIGHTBLUE,     "\033[94m");
  FGCOLORMAP(LIGHTMAGENTA,  "\033[95m");
  FGCOLORMAP(LIGHTCYAN,     "\033[96m");
  FGCOLORMAP(LIGHTWHITE,    "\033[97m");
  BGCOLORMAP(BLACK,         "\033[40m");
  BGCOLORMAP(RED,           "\033[41m");
  BGCOLORMAP(GREEN,         "\033[42m");
  BGCOLORMAP(YELLOW,        "\033[43m");
  BGCOLORMAP(BLUE,          "\033[44m");
  BGCOLORMAP(MAGENTA,       "\033[45m");
  BGCOLORMAP(CYAN,          "\033[46m");
  BGCOLORMAP(WHITE,         "\033[47m");
  BGCOLORMAP(GRAY,          "\033[100m");
  BGCOLORMAP(LIGHTRED,      "\033[101m");
  BGCOLORMAP(LIGHTGREEN,    "\033[102m");
  BGCOLORMAP(LIGHTYELLOW,   "\033[103m");
  BGCOLORMAP(LIGHTBLUE,     "\033[104m");
  BGCOLORMAP(LIGHTMAGENTA,  "\033[105m");
  BGCOLORMAP(LIGHTCYAN,     "\033[106m");
  BGCOLORMAP(LIGHTWHITE,    "\033[107m");
  default: break;
  }

#undef BGCOLORMAP
#undef FGCOLORMAP
#undef COLORMAP
  return "\033[00m";
}

inline std::ostream& set_colorful(std::ostream& stream, Color color) noexcept {
  return stream << get_colorful(color);
}

inline std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept {
}

inline std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept {
}

}
