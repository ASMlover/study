#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "bytecode/chunk.hh"
#include "runtime/gc.hh"
#include "runtime/module.hh"
#include "runtime/table.hh"
#include "runtime/value.hh"

namespace ms {

enum class InterpretResult {
  kOk = 0,
  kCompileError = 1,
  kRuntimeError = 2,
};

class Vm {
 public:
  Vm();

  void SetOutput(std::ostream& out);
  std::ostream& Output() const;

  InterpretResult Execute(const Chunk& chunk, std::string* error);
  InterpretResult ExecuteSource(const std::string& source, std::string* error);
  InterpretResult ExecuteModule(const std::string& source, std::shared_ptr<Module> module,
                                std::string* error);

  bool DefineGlobal(const std::string& name, Value value);
  bool GetGlobal(const std::string& name, Value* out) const;
  bool SetGlobal(const std::string& name, Value value);

  ModuleLoader& Modules();
  GcController& Gc();

 private:
  bool Push(Value value);
  bool Pop(Value* out);
  bool Peek(Value* out) const;
  bool ReadConstant(const Chunk& chunk, std::size_t ip, Constant* out) const;
  Value ConstantToValue(const Constant& constant) const;
  std::string LastSegment(const std::string& dotted) const;

  std::vector<Value> stack_;
  Table globals_;
  std::ostream* out_;
  ModuleLoader modules_;
  GcController gc_;
  std::shared_ptr<Module> current_module_;
};

}  // namespace ms
