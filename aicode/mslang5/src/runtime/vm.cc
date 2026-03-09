#include "runtime/vm.hh"

#include <cmath>
#include <iostream>

#include "bytecode/opcode.hh"
#include "frontend/compiler.hh"
#include "runtime/script_interpreter.hh"

namespace ms {

namespace {

InterpretResult LegacyResultFromError(const std::string& error) {
  if (ScriptInterpreter::IsCompileLikeError(error)) {
    return InterpretResult::kCompileError;
  }
  return InterpretResult::kRuntimeError;
}

std::string RuntimeError(const std::string& code, const std::string& message) {
  return "runtime error (" + code + "): " + message;
}

Diagnostic ParseWithFallback(const std::string& text, const std::string& phase,
                             const std::string& code, const std::string& file) {
  return ParseDiagnosticText(text, phase, code, file);
}

}  // namespace

Vm::Vm() : out_(&std::cout), gc_(1024 * 64) {}

void Vm::SetOutput(std::ostream& out) { out_ = &out; }

std::ostream& Vm::Output() const { return *out_; }

InterpretResult Vm::Execute(const Chunk& chunk, std::string* error) {
  last_diagnostics_.clear();
  stack_.clear();
  std::size_t ip = 0;
  while (ip < chunk.Code().size()) {
    const auto op = static_cast<OpCode>(chunk.Code()[ip++]);
    switch (op) {
      case OpCode::kConstant: {
        Constant c;
        if (!ReadConstant(chunk, ip++, &c)) {
          SetSingleDiagnostic(MakeDiagnostic("runtime", "MS4003", "invalid constant index",
                                             DiagnosticSpan{current_source_name_, 1}),
                              error);
          return InterpretResult::kRuntimeError;
        }
        Push(ConstantToValue(c));
        break;
      }
      case OpCode::kAdd:
      case OpCode::kSubtract:
      case OpCode::kMultiply:
      case OpCode::kDivide: {
        Value b;
        Value a;
        Pop(&b);
        Pop(&a);
        if (op == OpCode::kAdd && a.IsString() && b.IsString()) {
          Push(Value(a.AsString() + b.AsString()));
          break;
        }
        if (!a.IsNumber() || !b.IsNumber()) {
          SetSingleDiagnostic(
              ParseWithFallback(RuntimeError("MS4003", "operands must be numbers"), "runtime",
                                "MS4003", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        if (op == OpCode::kAdd) {
          Push(Value(a.AsNumber() + b.AsNumber()));
        } else if (op == OpCode::kSubtract) {
          Push(Value(a.AsNumber() - b.AsNumber()));
        } else if (op == OpCode::kMultiply) {
          Push(Value(a.AsNumber() * b.AsNumber()));
        } else {
          Push(Value(a.AsNumber() / b.AsNumber()));
        }
        break;
      }
      case OpCode::kNegate: {
        Value v;
        Pop(&v);
        if (!v.IsNumber()) {
          SetSingleDiagnostic(
              ParseWithFallback(RuntimeError("MS4003", "operand must be number"), "runtime",
                                "MS4003", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        Push(Value(-v.AsNumber()));
        break;
      }
      case OpCode::kPrint: {
        Value v;
        Pop(&v);
        Output() << v.ToString() << '\n';
        break;
      }
      case OpCode::kPop: {
        Value v;
        Pop(&v);
        break;
      }
      case OpCode::kDefineGlobal:
      case OpCode::kGetGlobal:
      case OpCode::kSetGlobal:
      case OpCode::kImportModule: {
        Constant c;
        if (!ReadConstant(chunk, ip++, &c) || !std::holds_alternative<std::string>(c)) {
          SetSingleDiagnostic(MakeDiagnostic("runtime", "MS4003",
                                             "global name must be string constant",
                                             DiagnosticSpan{current_source_name_, 1}),
                              error);
          return InterpretResult::kRuntimeError;
        }
        const std::string name = std::get<std::string>(c);
        if (op == OpCode::kDefineGlobal) {
          Value v;
          Pop(&v);
          DefineGlobal(name, v);
          break;
        }
        if (op == OpCode::kGetGlobal) {
          Value v;
          if (!GetGlobal(name, &v)) {
            SetSingleDiagnostic(ParseWithFallback(
                                    RuntimeError("MS4001", "undefined variable: " + name),
                                    "runtime", "MS4001", current_source_name_),
                                error);
            return InterpretResult::kRuntimeError;
          }
          Push(v);
          break;
        }
        if (op == OpCode::kSetGlobal) {
          Value v;
          Pop(&v);
          if (!SetGlobal(name, v)) {
            SetSingleDiagnostic(ParseWithFallback(
                                    RuntimeError("MS4001", "undefined variable: " + name),
                                    "runtime", "MS4001", current_source_name_),
                                error);
            return InterpretResult::kRuntimeError;
          }
          break;
        }
        std::string module_error;
        auto module = modules_.Load(name, *this, &module_error);
        if (!module) {
          SetSingleDiagnostic(ParseWithFallback(module_error, "module", "MS5004",
                                                current_source_name_),
                              error);
          return InterpretResult::kRuntimeError;
        }
        DefineGlobal(LastSegment(name), Value(module));
        break;
      }
      case OpCode::kImportSymbol: {
        Constant module_name;
        Constant symbol_name;
        Constant alias_name;
        if (!ReadConstant(chunk, ip++, &module_name) ||
            !ReadConstant(chunk, ip++, &symbol_name) ||
            !ReadConstant(chunk, ip++, &alias_name) ||
            !std::holds_alternative<std::string>(module_name) ||
            !std::holds_alternative<std::string>(symbol_name) ||
            !std::holds_alternative<std::string>(alias_name)) {
          SetSingleDiagnostic(
              MakeDiagnostic("runtime", "MS4003", "invalid from-import operand",
                             DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const std::string module = std::get<std::string>(module_name);
        const std::string symbol = std::get<std::string>(symbol_name);
        const std::string alias = std::get<std::string>(alias_name);

        std::string module_error;
        auto loaded = modules_.Load(module, *this, &module_error);
        if (!loaded) {
          SetSingleDiagnostic(ParseWithFallback(module_error, "module", "MS5004",
                                                current_source_name_),
                              error);
          return InterpretResult::kRuntimeError;
        }
        Value exported;
        if (!loaded->exports.Get(symbol, &exported)) {
          SetSingleDiagnostic(
              ParseWithFallback("module error (MS5002): module '" + module + "' has no symbol '" +
                                    symbol + "'",
                                "module", "MS5002", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        DefineGlobal(alias, exported);
        break;
      }
      case OpCode::kReturn:
        return InterpretResult::kOk;
    }
  }
  return InterpretResult::kOk;
}

InterpretResult Vm::ExecuteSource(const std::string& source, std::string* error) {
  return ExecuteSourceNamed(source, "script.ms", error);
}

InterpretResult Vm::ExecuteSourceNamed(const std::string& source, const std::string& source_name,
                                       std::string* error) {
  current_source_name_ = source_name;
  last_diagnostics_.clear();
  last_source_route_ = SourceExecutionRoute::kNone;

  auto execute_legacy = [&]() -> InterpretResult {
    last_source_route_ = SourceExecutionRoute::kLegacyInterpreter;
    std::string legacy_error;
    const bool ok = ScriptInterpreter::Execute(*this, source, &legacy_error);
    if (ok) {
      if (error != nullptr) {
        error->clear();
      }
      last_diagnostics_.clear();
      return InterpretResult::kOk;
    }
    SetSingleDiagnostic(ParseWithFallback(legacy_error, "runtime", "MS4003", source_name), error);
    return LegacyResultFromError(legacy_error);
  };

  if (source_mode_ == SourceExecutionMode::kLegacyOnly) {
    return execute_legacy();
  }

  CompileResult compiled = CompileToChunk(source);
  if (!compiled.errors.empty()) {
    std::vector<Diagnostic> diagnostics;
    diagnostics.reserve(compiled.errors.size());
    for (const auto& item : compiled.errors) {
      diagnostics.push_back(ParseWithFallback(item, "parse", "MS2001", source_name));
    }
    SetDiagnostics(std::move(diagnostics), error);
    if (source_mode_ == SourceExecutionMode::kVmPreferredWithLegacyFallback) {
      std::string legacy_error;
      if (ScriptInterpreter::Execute(*this, source, &legacy_error)) {
        last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
        if (error != nullptr) {
          error->clear();
        }
        last_diagnostics_.clear();
        return InterpretResult::kOk;
      }
      last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
      SetSingleDiagnostic(ParseWithFallback(legacy_error, "runtime", "MS4003", source_name), error);
      return LegacyResultFromError(legacy_error);
    }
    last_source_route_ = SourceExecutionRoute::kVmPipeline;
    return InterpretResult::kCompileError;
  }

  last_source_route_ = SourceExecutionRoute::kVmPipeline;
  const InterpretResult vm_result = Execute(compiled.chunk, error);
  if (vm_result != InterpretResult::kOk) {
    return vm_result;
  }
  return InterpretResult::kOk;
}

InterpretResult Vm::ExecuteModule(const std::string& source,
                                  std::shared_ptr<Module> module,
                                  std::string* error) {
  auto prev = current_module_;
  current_module_ = std::move(module);
  const std::string previous_source_name = current_source_name_;
  const std::string module_name = current_module_ != nullptr ? current_module_->name : "module.ms";
  const InterpretResult r = ExecuteSourceNamed(source, module_name, error);
  current_source_name_ = previous_source_name;
  current_module_ = prev;
  return r;
}

bool Vm::DefineGlobal(const std::string& name, Value value) {
  gc_.OnAllocation(name.size() + value.ToString().size());
  if (gc_.ShouldCollect()) {
    gc_.Collect();
  }
  globals_.Set(name, value);
  if (current_module_ != nullptr) {
    current_module_->exports.Set(name, value);
  }
  return true;
}

bool Vm::GetGlobal(const std::string& name, Value* out) const {
  return globals_.Get(name, out);
}

bool Vm::SetGlobal(const std::string& name, Value value) {
  if (!globals_.Contains(name)) {
    return false;
  }
  globals_.Set(name, value);
  if (current_module_ != nullptr) {
    current_module_->exports.Set(name, value);
  }
  return true;
}

ModuleLoader& Vm::Modules() { return modules_; }

GcController& Vm::Gc() { return gc_; }

void Vm::SetSourceExecutionMode(const SourceExecutionMode mode) { source_mode_ = mode; }

SourceExecutionMode Vm::GetSourceExecutionMode() const { return source_mode_; }

SourceExecutionRoute Vm::LastSourceExecutionRoute() const { return last_source_route_; }

const std::vector<Diagnostic>& Vm::LastDiagnostics() const { return last_diagnostics_; }

bool Vm::Push(Value value) {
  stack_.push_back(std::move(value));
  return true;
}

bool Vm::Pop(Value* out) {
  if (stack_.empty()) {
    return false;
  }
  if (out != nullptr) {
    *out = stack_.back();
  }
  stack_.pop_back();
  return true;
}

bool Vm::Peek(Value* out) const {
  if (stack_.empty()) {
    return false;
  }
  if (out != nullptr) {
    *out = stack_.back();
  }
  return true;
}

bool Vm::ReadConstant(const Chunk& chunk, const std::size_t ip, Constant* out) const {
  if (ip >= chunk.Code().size()) {
    return false;
  }
  const std::size_t index = chunk.Code()[ip];
  if (index >= chunk.Constants().size()) {
    return false;
  }
  if (out != nullptr) {
    *out = chunk.Constants()[index];
  }
  return true;
}

Value Vm::ConstantToValue(const Constant& constant) const {
  if (std::holds_alternative<std::monostate>(constant)) {
    return Value::Nil();
  }
  if (std::holds_alternative<bool>(constant)) {
    return Value(std::get<bool>(constant));
  }
  if (std::holds_alternative<double>(constant)) {
    return Value(std::get<double>(constant));
  }
  return Value(std::get<std::string>(constant));
}

std::string Vm::LastSegment(const std::string& dotted) const {
  const auto pos = dotted.find_last_of('.');
  if (pos == std::string::npos) {
    return dotted;
  }
  return dotted.substr(pos + 1);
}

void Vm::SetDiagnostics(std::vector<Diagnostic> diagnostics, std::string* error) {
  last_diagnostics_ = std::move(diagnostics);
  if (error != nullptr) {
    *error = RenderDiagnostics(last_diagnostics_);
  }
}

void Vm::SetSingleDiagnostic(const Diagnostic& diagnostic, std::string* error) {
  std::vector<Diagnostic> diagnostics;
  diagnostics.push_back(diagnostic);
  SetDiagnostics(std::move(diagnostics), error);
}

}  // namespace ms
