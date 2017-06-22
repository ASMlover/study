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
#include <Chaos/Types.h>
#include "trace_sweep.h"

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  constexpr int kCount = 1000;
  constexpr int kReleaseCount = 20;
  constexpr int kCreateCount = kReleaseCount * 3;
  for (auto i = 0; i < kCount; ++i) {
    for (auto j = 0; j < kCreateCount; ++j) {
      if ((j + 1) % 3 == 0) {
        auto* second = gc::TraceSweep::get_instance().release_object();
        auto* first = gc::TraceSweep::get_instance().release_object();
        gc::TraceSweep::get_instance().create_pair(first, second);
      }
      else {
        gc::TraceSweep::get_instance().create_int(i);
      }
    }

    for (auto j = 0; j < kReleaseCount; ++j)
      gc::TraceSweep::get_instance().release_object();
  }
  gc::TraceSweep::get_instance().collect_tracing();

  return 0;
}
