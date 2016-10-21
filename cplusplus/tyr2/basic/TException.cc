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
#include "TConfig.h"
#if !defined(TYR_WINDOWS)
# include <execinfo.h>
#endif
#include <stdlib.h>
#include "TException.h"

namespace tyr { namespace basic {

void Exception::fill_stack_trace(void) {
#if !defined(TYR_WINDOWS)
  const int LEN = 256;
  void* buffer[LEN];
  int nptrs = backtrace(buffer, LEN);
  char** strings = backtrace_symbols(buffer, nptrs);
  if (nullptr != strings) {
    for (int i = 0; i < nptrs; ++i) {
      stack_.append(strings[i]);
      stack_.push_back('\n');
    }
    free(strings);
  }
#endif
}

Exception::Exception(const char* what)
  : message_(what) {
  fill_stack_trace();
}

Exception::Exception(const std::string& what)
  : message_(what) {
  fill_stack_trace();
}

Exception::~Exception(void) throw() {
}

const char* Exception::what(void) const throw() {
  return message_.c_str();
}

const char* Exception::stack_trace(void) const throw() {
  return stack_.c_str();
}

}}
