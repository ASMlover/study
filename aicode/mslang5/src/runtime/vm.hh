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

  void set_output(std::ostream& out);
  std::ostream& output() const;

  InterpretResult execute(const Chunk& chunk, std::string* error);
  InterpretResult execute_source(const std::string& source, std::string* error);
  InterpretResult execute_source_named(const std::string& source, const std::string& source_name,
                                     std::string* error);
  InterpretResult execute_module(const std::string& source, std::shared_ptr<Module> module,
                                std::string* error);
  void set_source_execution_mode(SourceExecutionMode mode);
  SourceExecutionMode get_source_execution_mode() const;
  SourceExecutionRoute last_source_execution_route() const;
  const std::vector<Diagnostic>& last_diagnostics() const;

  bool define_global(const std::string& name, Value value);
  bool get_global(const std::string& name, Value* out) const;
  bool set_global(const std::string& name, Value value);

  ModuleLoader& modules();
  GcController& gc();

 private:
  bool push(Value value);
  bool pop(Value* out);
  bool peek(Value* out) const;
  bool read_constant(const Chunk& chunk, std::size_t ip, Constant* out) const;
  Value constant_to_value(const Constant& constant) const;
  std::string last_segment(const std::string& dotted) const;
  void set_diagnostics(std::vector<Diagnostic> diagnostics, std::string* error);
  void set_single_diagnostic(const Diagnostic& diagnostic, std::string* error);

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
