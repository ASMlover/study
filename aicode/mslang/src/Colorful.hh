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
#include "Types.hh"
#include "Macros.hh"

namespace ms {

enum class Color : int {
  kBlack,
  kRed,
  kGreen,
  kYellow,
  kBlue,
  kMagenta,
  kCyan,
  kWhite,
  kGray, // bright black
};

void set_foreground_colorful(std::ostream& os, Color c) noexcept;
void set_background_colorful(std::ostream& os, Color c) noexcept;
std::ostream& reset_colorful(std::ostream& os) noexcept;

using ColorFunc = void(*)(std::ostream&, Color);

struct ColorManip {
  Color color;
  ColorFunc func;
};

inline std::ostream& operator<<(std::ostream& os, const ColorManip& cm) {
  cm.func(os, cm.color);
  return os;
}

namespace fg {

inline ColorManip black   { Color::kBlack,   set_foreground_colorful };
inline ColorManip red     { Color::kRed,     set_foreground_colorful };
inline ColorManip green   { Color::kGreen,   set_foreground_colorful };
inline ColorManip yellow  { Color::kYellow,  set_foreground_colorful };
inline ColorManip blue    { Color::kBlue,    set_foreground_colorful };
inline ColorManip magenta { Color::kMagenta, set_foreground_colorful };
inline ColorManip cyan    { Color::kCyan,    set_foreground_colorful };
inline ColorManip white   { Color::kWhite,   set_foreground_colorful };
inline ColorManip gray    { Color::kGray,    set_foreground_colorful };

} // namespace fg

namespace bg {

inline ColorManip black   { Color::kBlack,   set_background_colorful };
inline ColorManip red     { Color::kRed,     set_background_colorful };
inline ColorManip green   { Color::kGreen,   set_background_colorful };
inline ColorManip yellow  { Color::kYellow,  set_background_colorful };
inline ColorManip blue    { Color::kBlue,    set_background_colorful };
inline ColorManip magenta { Color::kMagenta, set_background_colorful };
inline ColorManip cyan    { Color::kCyan,    set_background_colorful };
inline ColorManip white   { Color::kWhite,   set_background_colorful };
inline ColorManip gray    { Color::kGray,    set_background_colorful };

} // namespace bg

} // namespace ms
