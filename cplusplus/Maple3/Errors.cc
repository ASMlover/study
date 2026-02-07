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
#include "Common.hh"
#include "Errors.hh"

namespace ms {

str_t format_diagnostic(const str_t& message, const str_t& filename, int lineno, int colno, cstr_t level) noexcept {
  str_t origin;

  if (!filename.empty())
    origin = filename;
  else
    origin = "<input>";

  if (lineno > 0) {
    origin += ":";
    origin += as_string(lineno);
  }
  if (colno > 0) {
    origin += ":";
    origin += as_string(colno);
  }

  return as_string(origin, " ", level, ": ", message);
}

RuntimeError::RuntimeError(const str_t& message, const str_t& filename, int lineno, int colno) noexcept
  : std::runtime_error(format_diagnostic(message, filename, lineno, colno))
  , filename_{filename}
  , lineno_{lineno}
  , colno_{colno} {
}

ParseError::ParseError(const str_t& message, const str_t& filename, int lineno, int colno) noexcept
  : std::runtime_error(format_diagnostic(message, filename, lineno, colno))
  , filename_{filename}
  , lineno_{lineno}
  , colno_{colno} {
}

}
