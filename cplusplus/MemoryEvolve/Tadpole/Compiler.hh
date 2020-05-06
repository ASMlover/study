#pragma once

#include <Core/MEvolve.hh>

namespace _mevo::tadpole {

class VM;
class GlobalParser;
class FunctionObject;

class GlobalCompiler final : private UnCopyable {
  GlobalParser* gparser_{};
public:
  FunctionObject* compile(VM& vm, const str_t& source_bytes);
  void mark_compiler();
};

}