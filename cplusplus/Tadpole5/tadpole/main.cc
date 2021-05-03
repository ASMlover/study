// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
#include <fstream>
#include "common/colorful.hh"
#include "common/harness.hh"
#include "core/vm.hh"
#include "gc/gc.hh"

static int eval(tadpole::VM& vm, const std::string& source_bytes) {
  tadpole::InterpretRet r = vm.interpret(source_bytes);
  if (r == tadpole::InterpretRet::ECOMPILE)
    return -2;
  if (r == tadpole::InterpretRet::ERUNTIME)
    return -1;
  return 0;
}

int main(int argc, char* argv[]) {
  TADPOLE_UNUSED(argc), TADPOLE_UNUSED(argv);

#if defined(_TADPOLE_RUN_HARNESS)
  tadpole::harness::run_all_harness();
#endif

  tadpole::VM vm;
  if (argc < 2) {
    std::cout
      << tadpole::colorful::fg::lightgreen
      << "W E L C O M E   T O   T A D P O L E !!!"
      << tadpole::colorful::reset << std::endl;

    std::string line;
    for (;;) {
      if (!vm.is_running())
        break;

      std::cout << ">>> ";
      if (!std::getline(std::cin, line))
        break;

      eval(vm, line);

      tadpole::GC::get_instance().collect();
    }
  }
  else {
    std::fstream fp(argv[1]);
    if (fp.is_open()) {
      std::stringstream ss;
      ss << fp.rdbuf();

      if (int ec = eval(vm, ss.str()); ec != 0)
        std::exit(ec);
    }
    else {
      std::cerr << "ERROR: LOAD `" << argv[1] << "` FAILED !!!" << std::endl;
      std::exit(-1);
    }
  }

  return 0;
}
