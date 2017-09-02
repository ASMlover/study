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

#include <cstdint>

enum class ColorType : std::uint16_t {
  FORE_BLACK        = 0x00, // for posix \033[30m
  FORE_BLUE         = 0x01, // for posix \033[34m
  FORE_GREEN        = 0x02, // for posix \033[32m
  FORE_CYAN         = 0x03, // for posix \033[36m
  FORE_RED          = 0x04, // for posix \033[31m
  FORE_MAGENTA      = 0x05, // for posix \033[35m
  FORE_YELLOW       = 0x06, // for posix \033[33m
  FORE_WHITE        = 0x07, // for posix \033[37m
  FORE_GRAY         = 0x08, // for posix \033[90m
  FORE_LIGHTBLUE    = 0x09, // for posix \033[94m
  FORE_LIGHTGREEN   = 0x0A, // for posix \033[92m
  FORE_LIGHTCYAN    = 0x0B, // for posix \033[96m
  FORE_LIGHTRED     = 0x0C, // for posix \033[91m
  FORE_LIGHTMAGENTA = 0x0D, // for posix \033[95m
  FORE_LIGHTYELLOW  = 0x0E, // for posix \033[93m
  FORE_LIGHTWHITE   = 0x0F, // for posix \033[97m

  BACK_BLACK        = 0x00, // for posix \033[40m
  BACK_BLUE         = 0x10, // for posix \033[44m
  BACK_GREEN        = 0x20, // for posix \033[42m
  BACK_CYAN         = 0x30, // for posix \033[46m
  BACK_RED          = 0x40, // for posix \033[41m
  BACK_MAGENTA      = 0x50, // for posix \033[45m
  BACK_YELLOW       = 0x60, // for posix \033[43m
  BACK_WHITE        = 0x70, // for posix \033[47m
  BACK_GRAY         = 0x80, // for posix \033[100m
  BACK_LIGHTBLUE    = 0x90, // for posix \033[104m
  BACK_LIGHTGREEN   = 0xA0, // for posix \033[102m
  BACK_LIGHTCYAN    = 0xB0, // for posix \033[106m
  BACK_LIGHTRED     = 0xC0, // for posix \033[101m
  BACK_LIGHTMAGENTA = 0xD0, // for posix \033[105m
  BACK_LIGHTYELLOW  = 0xE0, // for posix \033[103m
  BACK_LIGHTWHITE   = 0xF0, // for posix \033[107m
};

int color_printf(ColorType color, const char* format, ...);
int color_printf(
    ColorType fore_color, ColorType back_color, const char* format, ...);
