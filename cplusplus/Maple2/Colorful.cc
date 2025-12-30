// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Macros.hh"
#if defined (MAPLE_MSVC)
# include <io.h>
# include <Windows.h>
#else
# include <unistd.h>
#endif
#include "Colorful.hh"

namespace ms {

inline FILE* get_standard_stream(const std::ostream& stream) noexcept {
  if (&stream == &std::cout)
    return stdout;
  else if (&stream == &std::cerr || &stream == &std::clog)
    return stderr;
  return nullptr;
}

inline bool is_atty(std::ostream& stream) noexcept {
  FILE* std_stream = get_standard_stream(stream);
  if (!std_stream)
    return false;

#if defined (MAPLE_MSVC)
  return ::_isatty(::_fileno(std_stream));
#else
  return ::isatty(::fileno(std_stream));
#endif
}

std::ostream& set_colorful(std::ostream& stream, Color color) noexcept { return stream; }
std::ostream& set_foreground_colorful(std::ostream& stream, Color color) noexcept { return stream; }
std::ostream& set_background_colorful(std::ostream& stream, Color color) noexcept { return stream; }

}
