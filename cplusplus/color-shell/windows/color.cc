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
#include <Windows.h>
#include <cstdint>
#include <mutex>
#include "../color.h"

namespace colorio {

static std::mutex g_mutex;

static std::uint16_t get_color(ColorType color) {
  switch (color) {
  case ColorType::FORE_BLACK:
    return FOREGROUND_RED & FOREGROUND_GREEN & FOREGROUND_BLUE;
  case ColorType::FORE_BLUE:
    return FOREGROUND_BLUE;
  case ColorType::FORE_LIGHTBLUE:
    return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case ColorType::FORE_GREEN:
    return FOREGROUND_GREEN;
  case ColorType::FORE_LIGHTGREEN:
    return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case ColorType::FORE_CYAN:
    return FOREGROUND_GREEN | FOREGROUND_BLUE;
  case ColorType::FORE_LIGHTCYAN:
    return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case ColorType::FORE_RED:
    return FOREGROUND_RED;
  case ColorType::FORE_LIGHTRED:
    return FOREGROUND_RED | FOREGROUND_INTENSITY;
  case ColorType::FORE_MAGENTA:
    return FOREGROUND_RED | FOREGROUND_BLUE;
  case ColorType::FORE_LIGHTMAGENTA:
    return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case ColorType::FORE_YELLOW:
    return FOREGROUND_RED | FOREGROUND_GREEN;
  case ColorType::FORE_LIGHTYELLOW:
    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case ColorType::FORE_WHITE:
    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  case ColorType::FORE_LIGHTWHITE:
    return FOREGROUND_RED |
      FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case ColorType::FORE_GRAY:
    return FOREGROUND_INTENSITY;
  case ColorType::BACK_BLACK:
    return BACKGROUND_RED & BACKGROUND_GREEN & BACKGROUND_BLUE;
  case ColorType::BACK_BLUE:
    return BACKGROUND_BLUE;
  case ColorType::BACK_LIGHTBLUE:
    return BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case ColorType::BACK_GREEN:
    return BACKGROUND_GREEN;
  case ColorType::BACK_LIGHTGREEN:
    return BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case ColorType::BACK_CYAN:
    return BACKGROUND_GREEN | BACKGROUND_BLUE;
  case ColorType::BACK_LIGHTCYAN:
    return BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case ColorType::BACK_RED:
    return BACKGROUND_RED;
  case ColorType::BACK_LIGHTRED:
    return BACKGROUND_RED | BACKGROUND_INTENSITY;
  case ColorType::BACK_MAGENTA:
    return BACKGROUND_RED | BACKGROUND_BLUE;
  case ColorType::BACK_LIGHTMAGENTA:
    return BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case ColorType::BACK_YELLOW:
    return BACKGROUND_RED | BACKGROUND_GREEN;
  case ColorType::BACK_LIGHTYELLOW:
    return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case ColorType::BACK_WHITE:
    return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
  case ColorType::BACK_LIGHTWHITE:
    return BACKGROUND_RED |
      BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case ColorType::BACK_GRAY:
    return BACKGROUND_INTENSITY;
  }
  return 0;
}

static int vfprintf_impl(std::FILE* stream,
    std::uint16_t color, const char* format, std::va_list ap) {
  HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(out_handle, &info);
  WORD old_color = info.wAttributes;

  int n{};
  {
    std::unique_lock<std::mutex> g(g_mutex);
    SetConsoleTextAttribute(out_handle, color);
    n = std::vfprintf(stream, format, ap);
    SetConsoleTextAttribute(out_handle, old_color);
  }

  return n;
}

int color_vfprintf(std::FILE* stream,
    ColorType color, const char* format, std::va_list ap) {
  return vfprintf_impl(stream, get_color(color), format, ap);
}

int color_vfprintf(std::FILE* stream,
    ColorType fore_color, ColorType back_color,
    const char* format, std::va_list ap) {
  return vfprintf_impl(stream,
      get_color(fore_color) | get_color(back_color), format, ap);
}

}
