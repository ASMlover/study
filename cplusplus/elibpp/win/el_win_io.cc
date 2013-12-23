//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "../elib_internal.h"
#include "../el_io.h"



namespace el {

static inline int 
ColorVfprintf(FILE* stream, int color, const char* format, va_list ap)
{
  HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(out_handle, &info);
  WORD old_color = info.wAttributes;

  switch (color) {
  case kColorTypeRed:
    color = FOREGROUND_INTENSITY | FOREGROUND_RED;
    break;
  case kColorTypeGreen:
    color = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
    break;
  default:
    color = old_color;
    break;
  }

  SetConsoleTextAttribute(out_handle, (WORD)color);
  int ret = vfprintf(stream, format, ap);
  SetConsoleTextAttribute(out_handle, old_color);

  return ret;
}




int 
ColorPrintf(int color, const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  int ret = ColorVfprintf(stdout, color, format, ap);
  va_end(ap);

  return ret;
}

int 
ColorFprintf(FILE* stream, int color, const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  int ret = ColorVfprintf(stream, color, format, ap);
  va_end(ap);

  return ret;
}

}
