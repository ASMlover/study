// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <Chaos/Except/SystemError.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/IO/ColorIO.h>

namespace Chaos {

namespace ColorIO {
  static Mutex g_color_mutex;

  int vfprintf(FILE* stream, ColorType color, const char* format, va_list ap) {
    HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(out_handle, &info);
    WORD old_color = info.wAttributes;
    WORD new_color = old_color;

    switch (color) {
    case ColorType::COLORTYPE_INVALID:
      __chaos_throw_exception(std::logic_error("invalid color type"));
      break;
    case ColorType::COLORTYPE_RED:
      new_color = FOREGROUND_INTENSITY | FOREGROUND_RED;
      break;
    case ColorType::COLORTYPE_GREEN:
      new_color = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
      break;
    }

    int n;
    {
      ScopedLock<Mutex> guard(g_color_mutex);
      SetConsoleTextAttribute(out_handle, new_color);
      n = ::vfprintf(stream, format, ap);
      SetConsoleTextAttribute(out_handle, old_color);
    }

    return n;
  }
}

}
