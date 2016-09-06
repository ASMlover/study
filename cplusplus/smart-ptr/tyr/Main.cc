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
#include "Tyr.h"
#include "TSmartPtr.h"

#define UNUSED(x) {x = x;}

static void TyrTrans(const tyr::WeakPtr<int>& wp) {
  tyr::SmartPtr<int> p = wp.Lock();
  if (p.Unique())
    std::cout << "unique value of `p` is: " << *p << std::endl;
  else
    std::cout << "non-unique value of `p` is: " << *p << std::endl;
}

static void TyrDemo(void) {
  tyr::SmartPtr<int> p(new int(56));
  std::cout << "value of `p` is: " << *p << ", unique is: " << p.Unique() << std::endl;

  tyr::WeakPtr<int> wp(p);
  TyrTrans(wp);
}

int main(int argc, char* argv[]) {
  UNUSED(argc)
  UNUSED(argv)

  TyrDemo();

  return 0;
}
