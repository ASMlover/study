// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <windows.h>
#include <iostream>
#include "Colorful.hh"

namespace ms {

namespace detail {

inline WORD color_to_fg_attr(Color c) noexcept {
  switch (c) {
  case Color::kBlack:   return 0;
  case Color::kRed:     return FOREGROUND_RED | FOREGROUND_INTENSITY;
  case Color::kGreen:   return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case Color::kYellow:  return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  case Color::kBlue:    return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Color::kMagenta: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Color::kCyan:    return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Color::kWhite:   return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
  case Color::kGray:    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  }
  return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
}

inline WORD color_to_bg_attr(Color c) noexcept {
  switch (c) {
  case Color::kBlack:   return 0;
  case Color::kRed:     return BACKGROUND_RED | BACKGROUND_INTENSITY;
  case Color::kGreen:   return BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case Color::kYellow:  return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
  case Color::kBlue:    return BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Color::kMagenta: return BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Color::kCyan:    return BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Color::kWhite:   return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
  case Color::kGray:    return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
  }
  return 0;
}

} // namespace detail

inline void set_foreground_colorful_impl(std::ostream& os, Color c) noexcept {
  MAPLE_UNUSED(os);
  HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(h, &info);
  WORD bg = info.wAttributes & 0xF0;
  SetConsoleTextAttribute(h, bg | detail::color_to_fg_attr(c));
}

inline void set_background_colorful_impl(std::ostream& os, Color c) noexcept {
  MAPLE_UNUSED(os);
  HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(h, &info);
  WORD fg = info.wAttributes & 0x0F;
  SetConsoleTextAttribute(h, fg | detail::color_to_bg_attr(c));
}

inline void reset_colorful_impl(std::ostream& os) noexcept {
  MAPLE_UNUSED(os);
  HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
  SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

} // namespace ms
