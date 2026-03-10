#include "runtime/vm.hh"

#include <cmath>
#include <iostream>

#include "bytecode/opcode.hh"
#include "frontend/compiler.hh"
#include "runtime/script_interpreter.hh"

namespace ms {

namespace {

InterpretResult LegacyResultFromError(const std::string& error) {
  if (ScriptInterpreter::is_compile_like_error(error)) {
    return InterpretResult::kCompileError;
  }
  return InterpretResult::kRuntimeError;
}

std::string RuntimeError(const std::string& code, const std::string& message) {
  return "runtime error (" + code + "): " + message;
}

Diagnostic ParseWithFallback(const std::string& text, const std::string& phase,
                             const std::string& code, const std::string& file) {
  return parse_diagnostic_text(text, phase, code, file);
}

}  // namespace

Vm::Vm() : out_(&std::cout), gc_(1024 * 64) {}

void Vm::set_output(std::ostream& out) noexcept { out_ = &out; }

std::ostream& Vm::output() const noexcept { return *out_; }

InterpretResult Vm::execute(const Chunk& chunk, std::string* error) {
  last_diagnostics_.clear();
  stack_.clear();
  std::size_t ip = 0;
  while (ip < chunk.code().size()) {
    const auto op = static_cast<OpCode>(chunk.code()[ip++]);
    switch (op) {
      case OpCode::kConstant: {
        Constant c;
        if (!read_constant(chunk, ip++, &c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid constant index",
                                             DiagnosticSpan{current_source_name_, 1}),
                              error);
          return InterpretResult::kRuntimeError;
        }
        push(constant_to_value(c));
        break;
      }
      case OpCode::kAdd:
      case OpCode::kSubtract:
      case OpCode::kMultiply:
      case OpCode::kDivide: {
        Value b;
        Value a;
        pop(&b);
        pop(&a);
        if (op == OpCode::kAdd && a.is_string() && b.is_string()) {
          push(Value(a.as_string() + b.as_string()));
          break;
        }
        if (!a.is_number() || !b.is_number()) {
          set_single_diagnostic(
              ParseWithFallback(RuntimeError("MS4003", "operands must be numbers"), "runtime",
                                "MS4003", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        if (op == OpCode::kAdd) {
          push(Value(a.as_number() + b.as_number()));
        } else if (op == OpCode::kSubtract) {
          push(Value(a.as_number() - b.as_number()));
        } else if (op == OpCode::kMultiply) {
          push(Value(a.as_number() * b.as_number()));
        } else {
          push(Value(a.as_number() / b.as_number()));
        }
        break;
      }
      case OpCode::kNegate: {
        Value v;
        pop(&v);
        if (!v.is_number()) {
          set_single_diagnostic(
              ParseWithFallback(RuntimeError("MS4003", "operand must be number"), "runtime",
                                "MS4003", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        push(Value(-v.as_number()));
        break;
      }
      case OpCode::kPrint: {
        Value v;
        pop(&v);
        output() << v.to_string() << '\n';
        break;
      }
      case OpCode::kPop: {
        Value v;
        pop(&v);
        break;
      }
      case OpCode::kDefineGlobal:
      case OpCode::kGetGlobal:
      case OpCode::kSetGlobal:
      case OpCode::kImportModule: {
        Constant c;
        if (!read_constant(chunk, ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                             "global name must be string constant",
                                             DiagnosticSpan{current_source_name_, 1}),
                              error);
          return InterpretResult::kRuntimeError;
        }
        const std::string name = std::get<std::string>(c);
        if (op == OpCode::kDefineGlobal) {
          Value v;
          pop(&v);
          define_global(name, v);
          break;
        }
        if (op == OpCode::kGetGlobal) {
          Value v;
          if (!get_global(name, &v)) {
            set_single_diagnostic(ParseWithFallback(
                                    RuntimeError("MS4001", "undefined variable: " + name),
                                    "runtime", "MS4001", current_source_name_),
                                error);
            return InterpretResult::kRuntimeError;
          }
          push(v);
          break;
        }
        if (op == OpCode::kSetGlobal) {
          Value v;
          pop(&v);
          if (!set_global(name, v)) {
            set_single_diagnostic(ParseWithFallback(
                                    RuntimeError("MS4001", "undefined variable: " + name),
                                    "runtime", "MS4001", current_source_name_),
                                error);
            return InterpretResult::kRuntimeError;
          }
          break;
        }
        std::string module_error;
        auto module = modules_.load(name, *this, &module_error);
        if (!module) {
          set_single_diagnostic(ParseWithFallback(module_error, "module", "MS5004",
                                                current_source_name_),
                              error);
          return InterpretResult::kRuntimeError;
        }
        define_global(last_segment(name), Value(module));
        break;
      }
      case OpCode::kImportSymbol: {
        Constant module_name;
        Constant symbol_name;
        Constant alias_name;
        if (!read_constant(chunk, ip++, &module_name) ||
            !read_constant(chunk, ip++, &symbol_name) ||
            !read_constant(chunk, ip++, &alias_name) ||
            !std::holds_alternative<std::string>(module_name) ||
            !std::holds_alternative<std::string>(symbol_name) ||
            !std::holds_alternative<std::string>(alias_name)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid from-import operand",
                             DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const std::string module = std::get<std::string>(module_name);
        const std::string symbol = std::get<std::string>(symbol_name);
        const std::string alias = std::get<std::string>(alias_name);

        std::string module_error;
        auto loaded = modules_.load(module, *this, &module_error);
        if (!loaded) {
          set_single_diagnostic(ParseWithFallback(module_error, "module", "MS5004",
                                                current_source_name_),
                              error);
          return InterpretResult::kRuntimeError;
        }
        Value exported;
        if (!loaded->exports.get(symbol, &exported)) {
          set_single_diagnostic(
              ParseWithFallback("module error (MS5002): module '" + module + "' has no symbol '" +
                                    symbol + "'",
                                "module", "MS5002", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        define_global(alias, exported);
        break;
      }
      case OpCode::kReturn:
        return InterpretResult::kOk;
    }
  }
  return InterpretResult::kOk;
}

InterpretResult Vm::execute_source(const std::string& source, std::string* error) {
  return execute_source_named(source, "script.ms", error);
}

InterpretResult Vm::execute_source_named(const std::string& source, const std::string& source_name,
                                       std::string* error) {
  current_source_name_ = source_name;
  last_diagnostics_.clear();
  last_source_route_ = SourceExecutionRoute::kNone;

  auto execute_legacy = [&]() -> InterpretResult {
    last_source_route_ = SourceExecutionRoute::kLegacyInterpreter;
    std::string legacy_error;
    const bool ok = ScriptInterpreter::execute(*this, source, &legacy_error);
    if (ok) {
      if (error != nullptr) {
        error->clear();
      }
      last_diagnostics_.clear();
      return InterpretResult::kOk;
    }
    set_single_diagnostic(ParseWithFallback(legacy_error, "runtime", "MS4003", source_name), error);
    return LegacyResultFromError(legacy_error);
  };

  if (source_mode_ == SourceExecutionMode::kLegacyOnly) {
    return execute_legacy();
  }

  CompileResult compiled = compile_to_chunk(source);
  if (!compiled.errors.empty()) {
    std::vector<Diagnostic> diagnostics;
    diagnostics.reserve(compiled.errors.size());
    for (const auto& item : compiled.errors) {
      diagnostics.push_back(ParseWithFallback(item, "parse", "MS2001", source_name));
    }
    set_diagnostics(std::move(diagnostics), error);
    if (source_mode_ == SourceExecutionMode::kVmPreferredWithLegacyFallback) {
      std::string legacy_error;
      if (ScriptInterpreter::execute(*this, source, &legacy_error)) {
        last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
        if (error != nullptr) {
          error->clear();
        }
        last_diagnostics_.clear();
        return InterpretResult::kOk;
      }
      last_source_route_ = SourceExecutionRoute::kVmCompileFailedThenLegacy;
      set_single_diagnostic(ParseWithFallback(legacy_error, "runtime", "MS4003", source_name), error);
      return LegacyResultFromError(legacy_error);
    }
    last_source_route_ = SourceExecutionRoute::kVmPipeline;
    return InterpretResult::kCompileError;
  }

  last_source_route_ = SourceExecutionRoute::kVmPipeline;
  const InterpretResult vm_result = execute(compiled.chunk, error);
  if (vm_result != InterpretResult::kOk) {
    return vm_result;
  }
  return InterpretResult::kOk;
}

InterpretResult Vm::execute_module(const std::string& source,
                                  std::shared_ptr<Module> module,
                                  std::string* error) {
  auto prev = current_module_;
  current_module_ = std::move(module);
  const std::string previous_source_name = current_source_name_;
  const std::string module_name = current_module_ != nullptr ? current_module_->name : "module.ms";
  const InterpretResult r = execute_source_named(source, module_name, error);
  current_source_name_ = previous_source_name;
  current_module_ = prev;
  return r;
}

bool Vm::define_global(const std::string& name, Value value) {
  gc_.on_allocation(name.size() + value.to_string().size());
  if (gc_.should_collect()) {
    gc_.collect();
  }
  globals_.set(name, value);
  if (current_module_ != nullptr) {
    current_module_->exports.set(name, value);
  }
  return true;
}

bool Vm::get_global(const std::string& name, Value* out) const {
  return globals_.get(name, out);
}

bool Vm::set_global(const std::string& name, Value value) {
  if (!globals_.contains(name)) {
    return false;
  }
  globals_.set(name, value);
  if (current_module_ != nullptr) {
    current_module_->exports.set(name, value);
  }
  return true;
}

ModuleLoader& Vm::modules() noexcept { return modules_; }

GcController& Vm::gc() noexcept { return gc_; }

void Vm::set_source_execution_mode(const SourceExecutionMode mode) noexcept {
  source_mode_ = mode;
}

SourceExecutionMode Vm::get_source_execution_mode() const noexcept {
  return source_mode_;
}

SourceExecutionRoute Vm::last_source_execution_route() const noexcept {
  return last_source_route_;
}

const std::vector<Diagnostic>& Vm::last_diagnostics() const noexcept {
  return last_diagnostics_;
}

bool Vm::push(Value value) {
  stack_.push_back(std::move(value));
  return true;
}

bool Vm::pop(Value* out) {
  if (stack_.empty()) {
    return false;
  }
  if (out != nullptr) {
    *out = stack_.back();
  }
  stack_.pop_back();
  return true;
}

bool Vm::peek(Value* out) const noexcept {
  if (stack_.empty()) {
    return false;
  }
  if (out != nullptr) {
    *out = stack_.back();
  }
  return true;
}

bool Vm::read_constant(const Chunk& chunk, const std::size_t ip, Constant* out) const noexcept {
  if (ip >= chunk.code().size()) {
    return false;
  }
  const std::size_t index = chunk.code()[ip];
  if (index >= chunk.constants().size()) {
    return false;
  }
  if (out != nullptr) {
    *out = chunk.constants()[index];
  }
  return true;
}

Value Vm::constant_to_value(const Constant& constant) const {
  if (std::holds_alternative<std::monostate>(constant)) {
    return Value::nil();
  }
  if (std::holds_alternative<bool>(constant)) {
    return Value(std::get<bool>(constant));
  }
  if (std::holds_alternative<double>(constant)) {
    return Value(std::get<double>(constant));
  }
  return Value(std::get<std::string>(constant));
}

std::string Vm::last_segment(const std::string& dotted) const {
  const auto pos = dotted.find_last_of('.');
  if (pos == std::string::npos) {
    return dotted;
  }
  return dotted.substr(pos + 1);
}

void Vm::set_diagnostics(std::vector<Diagnostic> diagnostics, std::string* error) {
  last_diagnostics_ = std::move(diagnostics);
  if (error != nullptr) {
    *error = render_diagnostics(last_diagnostics_);
  }
}

void Vm::set_single_diagnostic(const Diagnostic& diagnostic, std::string* error) {
  std::vector<Diagnostic> diagnostics;
  diagnostics.push_back(diagnostic);
  set_diagnostics(std::move(diagnostics), error);
}

}  // namespace ms
