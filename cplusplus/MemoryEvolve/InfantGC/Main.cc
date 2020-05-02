// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <ctime>
#include <Core/MEvolve.hh>
#include <InfantGC/IntObject.hh>
#include <InfantGC/PairObject.hh>
#include <InfantGC/VM.hh>

int main(int argc, char* argv[]) {
  _MEVO_UNUSED(argc), _MEVO_UNUSED(argv);

  auto& vm = _mevo::infant::VM::get_instance();

  std::srand((unsigned int)std::time(nullptr));
  for (int i = 0; i < 1000; ++i) {
    auto r = std::rand() % 100;
    if (r > 60) {
      vm.pop();
    }
    else if (r > 50) {
      auto* second = vm.peek(0);
      auto* first = vm.peek(1);
      auto* pair = _mevo::infant::PairObject::create(vm, first, second);
      vm.pop();
      vm.pop();
      vm.push(pair);
    }
    else {
      vm.push(_mevo::infant::IntObject::create(vm, r + i));
    }
  }
  vm.collect();

  return 0;
}
