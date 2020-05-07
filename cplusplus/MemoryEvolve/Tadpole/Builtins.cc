#include <chrono>
#include <iostream>
#include <Tadpole/VM.hh>
#include <Tadpole/Builtins.hh>

namespace _mevo::tadpole {

void register_builtins(VM& vm) {
  // fn print(args, ...)
  vm.define_native("print", [](int argc, Value* args) -> Value {
    for (int i = 0; i < argc; ++i)
      std::cout << args[i] << " ";
    std::cout << std::endl;
    return nullptr;
    });

  // fn clock()
  vm.define_native("clock", [](int argc, Value* args) -> Value {
    return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    });

  // fn exit()
  vm.define_native("exit", [&vm](int argc, Value* args) -> Value {
    vm.stop_vm();
    return nullptr;
    });
}

}