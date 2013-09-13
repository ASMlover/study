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
#include <sl_list.h>
#include "sl_test_header.h"



TEST_IMPL(list)
{
  sl::list_t<int> list;
  ASSERT(list.empty());
  ASSERT(0 == list.size());

  for (int i = 0; i < 5; ++i)
    list.push_back(i);
  ASSERT(!list.empty());
  ASSERT(5 == list.size());
  ASSERT(0 == list.front());
  ASSERT(4 == list.back());

  for (int i = 0; i < 5; ++i)
    list.push_front((i + 1) * 100);
  ASSERT(10 == list.size());
  ASSERT(500 == list.front());

  list.pop_back();
  ASSERT(3 == list.back());
  ASSERT(9 == list.size());

  list.pop_front();
  ASSERT(400 == list.front());
  ASSERT(8 == list.size());

  list.clear();
  ASSERT(list.empty());
}
