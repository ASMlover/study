// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
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
#include <chrono>
#include <iostream>
#include "colorful.hh"
#include "vm.hh"
#include "gc.hh"
#include "builtins.hh"

namespace tadpole {

void register_builtins(TadpoleVM& vm) noexcept {
  // fn help() -> Nil
  vm.define_native("help", [](sz_t, Value*) -> Value {
        std::cout
          << colorful::fg::green << "Welcome To Tadpole! This is the help utility."
          << colorful::reset << std::endl << std::endl;

          std::cout << "print(...)      Prints the values to stdout stream" << std::endl;
          std::cout << "exit()          Exit the Tadpole terminal" << std::endl;
          std::cout << "time()          Return the current time in seconds since the Epoch" << std::endl;
          std::cout << "clock()         Return the CPU time since the start of the process" << std::endl;
          std::cout << "get_count()     Return number of objects tracked by GC" << std::endl;
          std::cout << "gc_threshold()  Return the current collection threshold" << std::endl;
          std::cout << "gc_threshold(n) Sets the collection threshold" << std::endl;
          std::cout << "gc_collect()    Run a full collection" << std::endl;

          return nullptr;
      });

  // fn print(arg1: Value, arg2: Value, ...) -> Nil
  vm.define_native("print", [](sz_t nargs, Value* args) -> Value {
        for (sz_t i = 0; i < nargs; ++i)
          std::cout << args[i] << " ";
        std::cout << std::endl;

        return nullptr;
      });

  // fn exit() -> Nil
  vm.define_native("exit", [&vm](sz_t, Value*) -> Value {
        vm.terminate();
        return nullptr;
      });

  // fn time() -> Numeric
  //
  // @returns
  //    Return the current time in seconds since the Epoch.
  vm.define_native("time", [](sz_t, Value*) -> Value {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
      });

  // fn clock() -> Numeric
  //
  // @returns
  //    Return the current processor time as a Numeric  expressed in microseconds.
  vm.define_native("clock", [](sz_t, Value*) -> Value {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
      });

  // fn gc_count() -> Numeric
  //
  // @returns
  //    Return the current collection counts.
  vm.define_native("gc_count", [](sz_t, Value*) -> Value {
        return GC::get_instance().get_count();
      });

  // fn gc_threshold() -> Numeric
  //
  // @returns
  //    Return the current collection threshold.
  //
  //
  // fn gc_threshold(threshold: Numeric) -> Numeric
  //
  // @args
  //    threshold: Numeric -> Number of collection threshold will be setted.
  //
  // @returns
  //    Return the current collection threshold.
  vm.define_native("gc_threshold", [](sz_t nargs, Value* args) -> Value {
        if (nargs == 1 && args != nullptr && args[0].as_numeric())
          GC::get_instance().set_threshold(args[0].as_integer<sz_t>());
        return GC::get_instance().get_threshold();
      });

  // fn gc_collect() -> Nil
  vm.define_native("gc_collect", [](sz_t, Value*) -> Value {
        GC::get_instance().collect();
        return nullptr;
      });
}

}
