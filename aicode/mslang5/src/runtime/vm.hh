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
#include "support/source.hh"

namespace ms {

enum class InterpretResult {
  kOk = 0,
  kCompileError = 1,
  kRuntimeError = 2,
};

enum class SourceExecutionMode {
  kVmPreferred = 0,
  kVmPreferredWithLegacyFallback = 1,
  kLegacyOnly = 2,
};

enum class SourceExecutionRoute {
  kNone = 0,
  kVmPipeline = 1,
  kLegacyInterpreter = 2,
  kVmCompileFailedThenLegacy = 3,
};

class Vm {
 public:
  Vm();

  void SetOutput(std::ostream& out);
  std::ostream& Output() const;

  InterpretResult Execute(const Chunk& chunk, std::string* error);
  InterpretResult ExecuteSource(const std::string& source, std::string* error);
  InterpretResult ExecuteSourceNamed(const std::string& source, const std::string& source_name,
                                     std::string* error);
  InterpretResult ExecuteModule(const std::string& source, std::shared_ptr<Module> module,
                                std::string* error);
  void SetSourceExecutionMode(SourceExecutionMode mode);
  SourceExecutionMode GetSourceExecutionMode() const;
  SourceExecutionRoute LastSourceExecutionRoute() const;
  const std::vector<Diagnostic>& LastDiagnostics() const;

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
  void SetDiagnostics(std::vector<Diagnostic> diagnostics, std::string* error);
  void SetSingleDiagnostic(const Diagnostic& diagnostic, std::string* error);

  std::vector<Value> stack_;
  Table globals_;
  std::ostream* out_;
  ModuleLoader modules_;
  GcController gc_;
  std::shared_ptr<Module> current_module_;
  std::string current_source_name_ = "script.ms";
  std::vector<Diagnostic> last_diagnostics_;
  SourceExecutionMode source_mode_ = SourceExecutionMode::kVmPreferredWithLegacyFallback;
  SourceExecutionRoute last_source_route_ = SourceExecutionRoute::kNone;
};

}  // namespace ms
