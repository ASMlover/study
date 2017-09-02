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
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "color.h"

static int color_vfprintf(std::FILE* stream,
    WORD color, const char* format, std::va_list ap) {
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(output, &info);
  WORD old_color = info.wAttributes;

  SetConsoleTextAttribute(output, color);
  auto n = std::vfprintf(stream, format, ap);
  SetConsoleTextAttribute(output, old_color);

  return n;
}

int color_printf(ColorType color, const char* format, ...) {
  std::va_list ap;

  va_start(ap, format);
  auto n = color_vfprintf(stdout, static_cast<WORD>(color), format, ap);
  va_end(ap);

  return n;
}

int color_printf(
    ColorType fore_color, ColorType back_color, const char* format, ...) {
  if (fore_color < ColorType::FORE_BLACK
      || fore_color > ColorType::FORE_LIGHTWHITE)
    fore_color = ColorType::FORE_WHITE;
  if (back_color < ColorType::BACK_BLACK
      || back_color > ColorType::BACK_LIGHTWHITE)
    back_color = ColorType::BACK_BLACK;

  std::va_list ap;
  va_start(ap, format);
  auto n = color_vfprintf(stdout,
      static_cast<WORD>(fore_color) | static_cast<WORD>(back_color), format, ap);
  va_end(ap);

  return n;
}
