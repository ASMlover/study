// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <mutex>
#include "../color.h"

namespace colorio {

static std::mutex g_mutex;

static const char* get_color(ColorType color) {
  switch (color) {
  case ColorType::FORE_BLACK: return "\033[30m";
  case ColorType::FORE_BLUE: return "\033[34m";
  case ColorType::FORE_LIGHTBLUE: return "\033[94m";
  case ColorType::FORE_GREEN: return "\033[32m";
  case ColorType::FORE_LIGHTGREEN: return "\033[92m";
  case ColorType::FORE_CYAN: return "\033[36m";
  case ColorType::FORE_LIGHTCYAN: return "\033[96m";
  case ColorType::FORE_RED: return "\033[31m";
  case ColorType::FORE_LIGHTRED: return "\033[91m";
  case ColorType::FORE_MAGENTA: return "\033[35m";
  case ColorType::FORE_LIGHTMAGENTA: return "\033[95m";
  case ColorType::FORE_YELLOW: return "\033[33m";
  case ColorType::FORE_LIGHTYELLOW: return "\033[93m";
  case ColorType::FORE_WHITE: return "\033[37m";
  case ColorType::FORE_LIGHTWHITE: return "\033[97m";
  case ColorType::FORE_GRAY: return "\033[90m";
  case ColorType::BACK_BLACK: return "\033[40m";
  case ColorType::BACK_BLUE: return "\033[44m";
  case ColorType::BACK_LIGHTBLUE: return "\033[104m";
  case ColorType::BACK_GREEN: return "\033[42m";
  case ColorType::BACK_LIGHTGREEN: return "\033[102m";
  case ColorType::BACK_CYAN: return "\033[46m";
  case ColorType::BACK_LIGHTCYAN: return "\033[106m";
  case ColorType::BACK_RED: return "\033[41m";
  case ColorType::BACK_LIGHTRED: return "\033[101m";
  case ColorType::BACK_MAGENTA: return "\033[45m";
  case ColorType::BACK_LIGHTMAGENTA: return "\033[105m";
  case ColorType::BACK_YELLOW: return "\033[43m";
  case ColorType::BACK_LIGHTYELLOW: return "\033[103m";
  case ColorType::BACK_WHITE: return "\033[47m";
  case ColorType::BACK_LIGHTWHITE: return "\033[107m";
  case ColorType::BACK_GRAY: return "\033[100m";
  case ColorType::COLOR_COUNT: break;
  };
  return "\033[0m";
}

static int vfprintf_impl(std::FILE* stream,
    const char* color, const char* format, std::va_list ap) {
  std::unique_lock<std::mutex> g(g_mutex);

  std::fprintf(stream, "%s", color);
  auto n = std::vfprintf(stream, format, ap);
  std::fprintf(stream, "\033[0m");

  return n;
}

int color_vfprintf(std::FILE* stream,
    ColorType color, const char* format, std::va_list ap) {
  return vfprintf_impl(stream, get_color(color), format, ap);
}

int color_vfprintf(std::FILE* stream,
    ColorType fore_color, ColorType back_color,
    const char* format, std::va_list ap) {
  char color[64];
  std::snprintf(color,
      sizeof(color), "%s%s", get_color(fore_color), get_color(back_color));
  return vfprintf_impl(stream, color, format, ap);
}

}
