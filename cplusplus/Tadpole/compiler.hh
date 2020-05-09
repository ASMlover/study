#pragma once

#include "common.hh"

namespace tadpole {

class VM;
class FunctionObject;
class TadpoleParser;

class TadpoleCompiler final : private UnCopyable {
  TadpoleParser* tparser_{};
public:
  FunctionObject* compile(VM& vm, const str_t& source_bytes);
  void mark_compiler();
};

}