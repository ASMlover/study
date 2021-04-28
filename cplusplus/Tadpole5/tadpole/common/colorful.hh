// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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

#include "common.hh"

namespace tadpole::colorful {

#define SET_COLORFUL(c) set_colorful(stream, Colorful::c)

enum class Colorful : u8_t {
  RESET,
  FG_RED,
  FG_GREEN,
  FG_YELLOW,
  FG_BLUE,
  FG_MAGENTA,
  FG_CYAN,
  FG_WHITE,
  FG_GRAY,
};

std::ostream& set_colorful(std::ostream& stream, Colorful c) noexcept;

inline std::ostream& reset(std::ostream& stream) noexcept { return SET_COLORFUL(RESET); }
inline std::ostream& red(std::ostream& stream) noexcept { return SET_COLORFUL(FG_RED); }
inline std::ostream& green(std::ostream& stream) noexcept { return SET_COLORFUL(FG_GREEN); }
inline std::ostream& yellow(std::ostream& stream) noexcept { return SET_COLORFUL(FG_YELLOW); }
inline std::ostream& blue(std::ostream& stream) noexcept { return SET_COLORFUL(FG_BLUE); }
inline std::ostream& magenta(std::ostream& stream) noexcept { return SET_COLORFUL(FG_MAGENTA); }
inline std::ostream& cyan(std::ostream& stream) noexcept { return SET_COLORFUL(FG_CYAN); }
inline std::ostream& white(std::ostream& stream) noexcept { return SET_COLORFUL(FG_WHITE); }
inline std::ostream& gray(std::ostream& stream) noexcept { return SET_COLORFUL(FG_GRAY); }

}
