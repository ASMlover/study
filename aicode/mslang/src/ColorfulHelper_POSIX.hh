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

#include <iostream>
#include "Colorful.hh"

namespace ms {

inline void set_foreground_colorful_impl(std::ostream& os, Color c) noexcept {
  int code{};
  switch (c) {
  case Color::kBlack:   code = 30; break;
  case Color::kRed:     code = 31; break;
  case Color::kGreen:   code = 32; break;
  case Color::kYellow:  code = 33; break;
  case Color::kBlue:    code = 34; break;
  case Color::kMagenta: code = 35; break;
  case Color::kCyan:    code = 36; break;
  case Color::kWhite:   code = 37; break;
  case Color::kGray:    code = 90; break;
  }
  os << "\033[" << code << "m";
}

inline void set_background_colorful_impl(std::ostream& os, Color c) noexcept {
  int code{};
  switch (c) {
  case Color::kBlack:   code = 40; break;
  case Color::kRed:     code = 41; break;
  case Color::kGreen:   code = 42; break;
  case Color::kYellow:  code = 43; break;
  case Color::kBlue:    code = 44; break;
  case Color::kMagenta: code = 45; break;
  case Color::kCyan:    code = 46; break;
  case Color::kWhite:   code = 47; break;
  case Color::kGray:    code = 100; break;
  }
  os << "\033[" << code << "m";
}

inline void reset_colorful_impl(std::ostream& os) noexcept {
  os << "\033[0m";
}

} // namespace ms
