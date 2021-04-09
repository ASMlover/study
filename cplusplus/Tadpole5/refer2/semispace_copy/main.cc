// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "common.hh"
#include "object.hh"
#include "semispace_copy.hh"

int main(int argc, char* argv[]) {
  TADPOLE_UNUSED(argc), TADPOLE_UNUSED(argv);

  int kCount = 1000;
  int kReleaseCount = 20;
  int kCreateCount = kReleaseCount * 3;

  for (int i = 0; i < kCount; ++i) {
    for (int j = 0; j < kCreateCount; ++j) {
      if ((j + 1) % 3 == 0) {
        auto* second = tadpole::gc::SemispaceCopy::get_instance().fetch_object();
        auto* first = tadpole::gc::SemispaceCopy::get_instance().fetch_object();
        tadpole::gc::SemispaceCopy::get_instance().create_object<tadpole::gc::PairObject>(first, second);
      }
      else {
        tadpole::gc::SemispaceCopy::get_instance().create_object<tadpole::gc::IntObject>(i * j);
      }
    }

    std::srand((unsigned int)std::time(nullptr));
    for (int j = 0; j < kReleaseCount; ++j) {
      if (std::rand() % 100 <= 90)
        tadpole::gc::SemispaceCopy::get_instance().fetch_object();
    }
  }
  tadpole::gc::SemispaceCopy::get_instance().collect();

  return 0;
}
