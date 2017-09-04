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
#include <iostream>
#include "color.h"

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  static const char* color_names[] = {
    "colorio::ColorType::FORE_BLACK",
    "colorio::ColorType::FORE_BLUE",
    "colorio::ColorType::FORE_GREEN",
    "colorio::ColorType::FORE_CYAN",
    "colorio::ColorType::FORE_RED",
    "colorio::ColorType::FORE_MAGENTA",
    "colorio::ColorType::FORE_YELLOW",
    "colorio::ColorType::FORE_WHITE",
    "colorio::ColorType::FORE_GRAY",
    "colorio::ColorType::FORE_LIGHTBLUE",
    "colorio::ColorType::FORE_LIGHTGREEN",
    "colorio::ColorType::FORE_LIGHTCYAN",
    "colorio::ColorType::FORE_LIGHTRED",
    "colorio::ColorType::FORE_LIGHTMAGENTA",
    "colorio::ColorType::FORE_LIGHTYELLOW",
    "colorio::ColorType::FORE_LIGHTWHITE",
    "colorio::ColorType::BACK_BLACK",
    "colorio::ColorType::BACK_BLUE",
    "colorio::ColorType::BACK_GREEN",
    "colorio::ColorType::BACK_CYAN",
    "colorio::ColorType::BACK_RED",
    "colorio::ColorType::BACK_MAGENTA",
    "colorio::ColorType::BACK_YELLOW",
    "colorio::ColorType::BACK_WHITE",
    "colorio::ColorType::BACK_GRAY",
    "colorio::ColorType::BACK_LIGHTBLUE",
    "colorio::ColorType::BACK_LIGHTGREEN",
    "colorio::ColorType::BACK_LIGHTCYAN",
    "colorio::ColorType::BACK_LIGHTRED",
    "colorio::ColorType::BACK_LIGHTMAGENTA",
    "colorio::ColorType::BACK_LIGHTYELLOW",
    "colorio::ColorType::BACK_LIGHTWHITE",
  };

  int color_count = static_cast<int>(colorio::ColorType::COLOR_COUNT);
  for (int i = 0; i < color_count; ++i) {
    auto c = static_cast<colorio::ColorType>(i);
    if (c == colorio::ColorType::FORE_BLACK)
      color_printf(c, colorio::ColorType::BACK_GRAY, "%s", color_names[i]);
    else if (c == colorio::ColorType::BACK_BLACK)
      color_printf(colorio::ColorType::FORE_GRAY, c, "%s", color_names[i]);
    else
      color_printf(c, "%s", color_names[i]);
    std::cout << std::endl;
  }

  int fc_beg = static_cast<int>(colorio::ColorType::FORE_BLACK);
  int fc_end = static_cast<int>(colorio::ColorType::FORE_LIGHTWHITE);
  int bc_beg = static_cast<int>(colorio::ColorType::BACK_BLACK);
  color_count = fc_end - fc_beg + 1;
  for (int i = 0; i < color_count; ++i) {
    for (int j = 0; j < color_count; ++j) {
      if (i == j)
        continue;
      auto fc = static_cast<colorio::ColorType>(fc_beg + i);
      auto bc = static_cast<colorio::ColorType>(bc_beg + j);
      color_printf(fc, bc,
          "%s - %s", color_names[fc_beg + i], color_names[bc_beg + j]);
      std::cout << std::endl;
    }
  }

  return 0;
}
