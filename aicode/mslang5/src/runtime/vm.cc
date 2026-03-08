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

}  // namespace

Vm::Vm() : out_(&std::cout), gc_(1024 * 64) {}

void Vm::SetOutput(std::ostream& out) { out_ = &out; }

std::ostream& Vm::Output() const { return *out_; }

InterpretResult Vm::Execute(const Chunk& chunk, std::string* error) {
  stack_.clear();
  std::size_t ip = 0;
  while (ip < chunk.Code().size()) {
    const auto op = static_cast<OpCode>(chunk.Code()[ip++]);
    switch (op) {
      case OpCode::kConstant: {
        Constant c;
        if (!ReadConstant(chunk, ip++, &c)) {
          if (error != nullptr) {
            *error = "invalid constant index";
          }
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
          if (error != nullptr) {
            *error = RuntimeError("MS4003", "operands must be numbers");
          }
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
          if (error != nullptr) {
            *error = RuntimeError("MS4003", "operand must be number");
          }
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
          if (error != nullptr) {
            *error = "global name must be string constant";
          }
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
            if (error != nullptr) {
              *error = RuntimeError("MS4001", "undefined variable: " + name);
            }
            return InterpretResult::kRuntimeError;
          }
          Push(v);
          break;
        }
        if (op == OpCode::kSetGlobal) {
          Value v;
          Pop(&v);
          if (!SetGlobal(name, v)) {
            if (error != nullptr) {
              *error = RuntimeError("MS4001", "undefined variable: " + name);
            }
            return InterpretResult::kRuntimeError;
          }
          break;
        }
        std::string module_error;
        auto module = modules_.Load(name, *this, &module_error);
        if (!module) {
          if (error != nullptr) {
            *error = module_error;
          }
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
          if (error != nullptr) {
            *error = "invalid from-import operand";
          }
          return InterpretResult::kRuntimeError;
        }
        const std::string module = std::get<std::string>(module_name);
        const std::string symbol = std::get<std::string>(symbol_name);
        const std::string alias = std::get<std::string>(alias_name);

        std::string module_error;
        auto loaded = modules_.Load(module, *this, &module_error);
        if (!loaded) {
          if (error != nullptr) {
            *error = module_error;
          }
          return InterpretResult::kRuntimeError;
        }
        Value exported;
        if (!loaded->exports.Get(symbol, &exported)) {
          if (error != nullptr) {
            *error = "module error (MS5002): module '" + module + "' has no symbol '" + symbol + "'";
          }
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
  last_source_route_ = SourceExecutionRoute::kNone;

  auto execute_legacy = [&]() -> InterpretResult {
    last_source_route_ = SourceExecutionRoute::kLegacyInterpreter;
    std::string legacy_error;
    const bool ok = ScriptInterpreter::Execute(*this, source, &legacy_error);
    if (ok) {
      if (error != nullptr) {
        error->clear();
      }
      return InterpretResult::kOk;
    }
    if (error != nullptr) {
      *error = legacy_error;
    }
    return LegacyResultFromError(legacy_error);
  };

  if (source_mode_ == SourceExecutionMode::kLegacyOnly) {
    return execute_legacy();
  }

  CompileResult compiled = CompileToChunk(source);
  if (!compiled.errors.empty()) {
    if (error != nullptr) {
      *error = compiled.errors.front();
    }
    if (source_mode_ == SourceExecutionMode::kVmPreferredWithLegacyFallback) {
      std::string legacy_error;
      if (ScriptInterpreter::Execute(*this, source, &legacy_error)) {
        last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
        if (error != nullptr) {
          error->clear();
        }
        return InterpretResult::kOk;
      }
      last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
      if (error != nullptr) {
        *error = legacy_error;
      }
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
  const InterpretResult r = ExecuteSource(source, error);
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

}  // namespace ms
