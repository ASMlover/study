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
#include <stdio.h>
#include "sl_test_header.h"

test_framework_t::test_framework_t(void)
{
}

test_framework_t::~test_framework_t(void)
{
}

test_framework_t& 
test_framework_t::singleton(void)
{
  static test_framework_t _s_test;

  return _s_test;
}

void 
test_framework_t::run(void)
{
  fprintf(stdout, "begin testing for slib++ ...\n");

  size_t size = case_list_.size();
  for (size_t i = 0; i < size; ++i) {
    fprintf(stdout, "\ttesting for %s module : \n", case_list_[i].name_);
    case_list_[i].test_();
    fprintf(stdout, "\ttesting passed !!!\n\n");
  }
  
  fprintf(stdout, "end testing for slib++ ...\n");
}

bool 
test_framework_t::register_test(const char* name, void (*test)(void))
{
  if (NULL == name || NULL == test)
    return false;

  case_list_.push_back(test_case_t(name, test));
  return true;
}



int 
main(int argc, char* argv[])
{
  test_framework_t::singleton().run();

  return 0;
}
