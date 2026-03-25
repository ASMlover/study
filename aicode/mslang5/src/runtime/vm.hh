#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "bytecode/chunk.hh"
#include "runtime/gc.hh"
#include "runtime/module.hh"
#include "runtime/object.hh"
#include "runtime/table.hh"
#include "runtime/value.hh"
#include "support/source.hh"

namespace ms {

enum class InterpretResult {
  kOk = 0,
  kCompileError = 1,
  kRuntimeError = 2,
};

enum class SourceExecutionRoute {
  kVmPipeline = 0,
};

class Vm {
 public:
  Vm();

  void set_output(std::ostream& out) noexcept;
  std::ostream& output() const noexcept;

  InterpretResult execute(const Chunk& chunk, std::string* error);
  InterpretResult execute_source(const std::string& source, std::string* error);
  InterpretResult execute_source_named(const std::string& source, const std::string& source_name,
                                       std::string* error);
  InterpretResult execute_module(const std::string& source, std::shared_ptr<Module> module,
                                 std::string* error);
  SourceExecutionRoute last_source_execution_route() const noexcept;
  const std::vector<Diagnostic>& last_diagnostics() const noexcept;

  bool define_global(const std::string& name, Value value);
  bool get_global(const std::string& name, Value* out) const;
  bool set_global(const std::string& name, Value value);

  ModuleLoader& modules() noexcept;
  GcController& gc() noexcept;
  std::shared_ptr<Module> load_standard_module(const std::string& module_name,
                                               std::string* error);

 private:
  struct CallFrame {
    ClosureObject* closure = nullptr;
    std::size_t ip = 0;
    std::size_t slot_base = 0;
  };

  bool push(Value value);
  bool pop(Value* out);
  bool peek(Value* out) const noexcept;
  bool read_constant(const Chunk& chunk, std::size_t ip, Constant* out) const noexcept;
  bool read_jump_offset(const Chunk& chunk, std::size_t ip, std::uint16_t* out) const noexcept;
  bool is_falsey(const Value& value) const noexcept;
  Value constant_to_value(const Constant& constant);
  std::string last_segment(const std::string& dotted) const;
  std::size_t line_for_instruction(const Chunk& chunk, std::size_t instruction_ip) const noexcept;
  DiagnosticSpan current_runtime_span() const;
  Diagnostic make_runtime_diagnostic(const std::string& code, const std::string& message) const;
  Diagnostic parse_diagnostic_with_current_span(const std::string& text,
                                                const std::string& phase,
                                                const std::string& code) const;
  void set_diagnostics(std::vector<Diagnostic> diagnostics, std::string* error);
  void set_single_diagnostic(const Diagnostic& diagnostic, std::string* error);

  bool call_closure(ClosureObject* closure, int arg_count,
                    std::vector<CallFrame>* frames, std::string* error);
  bool invoke_from_class(ClassObject* klass, const std::string& name,
                         int arg_count, std::vector<CallFrame>* frames, std::string* error);
  bool invoke_value(const Value& receiver, const std::string& name, int arg_count,
                    std::vector<CallFrame>* frames, std::string* error);
  bool bind_method(ClassObject* klass, const std::string& name,
                   const Value& receiver, std::string* error);
  bool call_value_at(std::size_t callee_index, int arg_count, std::vector<CallFrame>* frames,
                     std::string* error);
  UpvalueObject* capture_upvalue(std::size_t stack_index);
  void close_upvalues(std::size_t min_stack_index);
  Value read_upvalue(const UpvalueObject* upvalue) const;
  void write_upvalue(UpvalueObject* upvalue, Value value);

  void maybe_collect_garbage();
  void prune_untracked_owned_objects();
  void register_object_allocation(const std::shared_ptr<RuntimeObject>& object);
  void register_module_allocation(const std::shared_ptr<Module>& module);
  std::size_t estimate_object_bytes(const RuntimeObject* object) const;
  std::size_t estimate_module_bytes(const std::shared_ptr<Module>& module) const;
  RuntimeObject* create_native_object(const std::string& name, int arity,
                                      NativeCallable callable);
  bool register_native(const std::string& name, int arity, NativeCallable callable);
  bool install_std_io_exports(Module* module);
  bool install_std_math_exports(Module* module);
  bool install_std_str_exports(Module* module);
  bool install_std_time_exports(Module* module);
  bool install_std_debug_exports(Module* module);
  void install_core_natives();

  void trace_gc_roots(GcController& gc) const;
  void trace_gc_table(const Table& table, GcController& gc,
                      std::unordered_set<const void*>* seen_modules,
                      std::unordered_set<const void*>* seen_objects) const;
  void trace_gc_value(const Value& value, GcController& gc,
                      std::unordered_set<const void*>* seen_modules,
                      std::unordered_set<const void*>* seen_objects) const;
  void trace_gc_module(const std::shared_ptr<Module>& module, GcController& gc,
                       std::unordered_set<const void*>* seen_modules,
                       std::unordered_set<const void*>* seen_objects) const;
  void trace_gc_object(RuntimeObject* object, GcController& gc,
                       std::unordered_set<const void*>* seen_modules,
                       std::unordered_set<const void*>* seen_objects) const;

  std::vector<Value> stack_;
  std::vector<UpvalueObject*> open_upvalues_;
  std::vector<ClosureObject*> gc_frame_roots_;
  std::vector<std::shared_ptr<RuntimeObject>> gc_owned_objects_;
  std::vector<std::unique_ptr<NativeFunctionObject>> native_owned_objects_;
  Table globals_;
  std::ostream* out_;
  ModuleLoader modules_;
  GcController gc_;
  std::shared_ptr<Module> current_module_;
  std::string current_source_name_ = "script.ms";
  std::size_t current_runtime_line_ = 1;
  std::vector<Diagnostic> last_diagnostics_;
  SourceExecutionRoute last_source_route_ = SourceExecutionRoute::kVmPipeline;
};

}  // namespace ms