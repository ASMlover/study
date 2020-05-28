#include <chrono>
#include <ctime>
#include <iostream>
#include "vm.hh"
#include "builtins.hh"

namespace tadpole {

void register_builtins(VM& vm) {
  // fn print(arg1: Value, arg2: Value, ...) -> nil
  vm.define_native("print", [](sz_t argc, Value* args) -> Value {
      for (sz_t i = 0; i < argc; ++i)
        std::cout << args[i] << " ";
      std::cout << std::endl;

      return nullptr;
    });

  // fn exit() -> nil
  vm.define_native("exit", [&vm](sz_t, Value*) -> Value {
      vm.terminate();
      return nullptr;
    });

  // fn time() -> numeric
  //  UNIT: s
  vm.define_native("time", [](sz_t, Value*) -> Value {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    });

  // fn clock() -> numeric
  //  UNIT: us
  vm.define_native("clock", [](sz_t, Value*) -> Value {
      return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    });
}

}