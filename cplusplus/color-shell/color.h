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
#pragma once

#include <cstdarg>
#include <cstdio>

namespace colorio {

enum class ColorType {
  FORE_BLACK,
  FORE_BLUE,
  FORE_GREEN,
  FORE_CYAN,
  FORE_RED,
  FORE_MAGENTA,
  FORE_YELLOW,
  FORE_WHITE,
  FORE_GRAY,
  FORE_LIGHTBLUE,
  FORE_LIGHTGREEN,
  FORE_LIGHTCYAN,
  FORE_LIGHTRED,
  FORE_LIGHTMAGENTA,
  FORE_LIGHTYELLOW,
  FORE_LIGHTWHITE,

  BACK_BLACK,
  BACK_BLUE,
  BACK_GREEN,
  BACK_CYAN,
  BACK_RED,
  BACK_MAGENTA,
  BACK_YELLOW,
  BACK_WHITE,
  BACK_GRAY,
  BACK_LIGHTBLUE,
  BACK_LIGHTGREEN,
  BACK_LIGHTCYAN,
  BACK_LIGHTRED,
  BACK_LIGHTMAGENTA,
  BACK_LIGHTYELLOW,
  BACK_LIGHTWHITE,

  COLOR_COUNT,
};

int color_vfprintf(std::FILE* stream,
    ColorType color, const char* format, std::va_list ap);
int color_vfprintf(std::FILE* stream,
    ColorType fore_color, ColorType back_color,
    const char* format, std::va_list ap);

int color_printf(ColorType color, const char* format, ...);
int color_printf(
    ColorType fore_color, ColorType back_color, const char* format, ...);

}
