#include "runtime/vm.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <utility>

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

bool ShouldFallbackAfterCompileError(const std::vector<Diagnostic>& diagnostics) {
  if (diagnostics.empty()) {
    return false;
  }
  for (const auto& diagnostic : diagnostics) {
    if (diagnostic.phase != "parse" || diagnostic.code != "MS2001") {
      return false;
    }
  }
  return true;
}

}  // namespace

Vm::Vm() : out_(&std::cout), gc_(1024 * 64) {}

void Vm::set_output(std::ostream& out) noexcept { out_ = &out; }

std::ostream& Vm::output() const noexcept { return *out_; }

InterpretResult Vm::execute(const Chunk& chunk, std::string* error) {
  last_diagnostics_.clear();
  stack_.clear();
  open_upvalues_.clear();

  auto script_proto = std::make_shared<FunctionPrototype>();
  script_proto->name = "<script>";
  script_proto->chunk = std::make_shared<Chunk>(chunk);
  auto script_fn = std::make_shared<FunctionObject>(std::move(script_proto));
  auto script_closure = std::make_shared<ClosureObject>(std::move(script_fn));

  std::vector<CallFrame> frames;
  frames.push_back(CallFrame{script_closure, 0, 0});

  while (!frames.empty()) {
    CallFrame& frame = frames.back();
    if (frame.closure == nullptr || frame.closure->function == nullptr ||
        frame.closure->function->prototype == nullptr ||
        frame.closure->function->prototype->chunk == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "invalid call frame",
                          DiagnosticSpan{current_source_name_, 1}),
          error);
      return InterpretResult::kRuntimeError;
    }

    const Chunk& active_chunk = *frame.closure->function->prototype->chunk;
    if (frame.ip >= active_chunk.code().size()) {
      return InterpretResult::kOk;
    }

    const auto op = static_cast<OpCode>(active_chunk.code()[frame.ip++]);
    switch (op) {
      case OpCode::kConstant: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid constant index",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        push(constant_to_value(c));
        break;
      }
      case OpCode::kClosure: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) ||
            !std::holds_alternative<std::shared_ptr<FunctionPrototype>>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "closure operand must be function prototype",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const auto prototype = std::get<std::shared_ptr<FunctionPrototype>>(c);
        auto function = std::make_shared<FunctionObject>(prototype);
        auto closure = std::make_shared<ClosureObject>(function);
        closure->upvalues.resize(static_cast<std::size_t>(prototype->upvalue_count));

        for (int i = 0; i < prototype->upvalue_count; ++i) {
          if (frame.ip + 1 >= active_chunk.code().size()) {
            set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid closure operand",
                                                  DiagnosticSpan{current_source_name_, 1}),
                                  error);
            return InterpretResult::kRuntimeError;
          }
          const std::uint8_t is_local = active_chunk.code()[frame.ip++];
          const std::uint8_t index = active_chunk.code()[frame.ip++];
          if (is_local != 0) {
            closure->upvalues[static_cast<std::size_t>(i)] =
                capture_upvalue(frame.slot_base + static_cast<std::size_t>(index));
            continue;
          }
          if (static_cast<std::size_t>(index) >= frame.closure->upvalues.size()) {
            set_single_diagnostic(
                make_diagnostic("runtime", "MS4003", "upvalue index out of range",
                                DiagnosticSpan{current_source_name_, 1}),
                error);
            return InterpretResult::kRuntimeError;
          }
          closure->upvalues[static_cast<std::size_t>(i)] =
              frame.closure->upvalues[static_cast<std::size_t>(index)];
        }

        push(Value(std::static_pointer_cast<RuntimeObject>(closure)));
        break;
      }
      case OpCode::kEqual: {
        Value b;
        Value a;
        pop(&b);
        pop(&a);
        push(Value(a == b));
        break;
      }
      case OpCode::kGreater:
      case OpCode::kLess: {
        Value b;
        Value a;
        pop(&b);
        pop(&a);
        if (!a.is_number() || !b.is_number()) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "operands must be numbers"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        push(Value(op == OpCode::kGreater ? a.as_number() > b.as_number()
                                          : a.as_number() < b.as_number()));
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
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "operands must be numbers"),
                                                  "runtime", "MS4003", current_source_name_),
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
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "operand must be number"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        push(Value(-v.as_number()));
        break;
      }
      case OpCode::kNot: {
        Value v;
        pop(&v);
        push(Value(is_falsey(v)));
        break;
      }
      case OpCode::kPrint: {
        Value v;
        pop(&v);
        output() << v.to_string() << '\n';
        break;
      }
      case OpCode::kPop: {
        Value ignored;
        pop(&ignored);
        break;
      }
      case OpCode::kGetLocal:
      case OpCode::kSetLocal: {
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid local slot operand",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t slot = active_chunk.code()[frame.ip++];
        const std::size_t absolute_slot = frame.slot_base + slot;
        if (absolute_slot >= stack_.size()) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4001", "undefined local slot"),
                                                  "runtime", "MS4001", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        if (op == OpCode::kGetLocal) {
          push(stack_[absolute_slot]);
        } else {
          Value value;
          pop(&value);
          stack_[absolute_slot] = value;
        }
        break;
      }
      case OpCode::kGetUpvalue:
      case OpCode::kSetUpvalue: {
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid upvalue operand",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t slot = active_chunk.code()[frame.ip++];
        if (slot >= frame.closure->upvalues.size() || frame.closure->upvalues[slot] == nullptr) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "upvalue slot out of range",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        if (op == OpCode::kGetUpvalue) {
          push(read_upvalue(frame.closure->upvalues[slot]));
        } else {
          Value value;
          pop(&value);
          write_upvalue(frame.closure->upvalues[slot], value);
        }
        break;
      }
      case OpCode::kCall: {
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid call operand",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on call",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t callee_index = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
        if (!call_value_at(callee_index, arg_count, &frames, error)) {
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OpCode::kInvoke: {
        Constant method_name;
        if (!read_constant(active_chunk, frame.ip++, &method_name) ||
            !std::holds_alternative<std::string>(method_name)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid invoke method operand",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid invoke argument operand",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on invoke",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t receiver_index = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
        if (!invoke_value(stack_[receiver_index], std::get<std::string>(method_name), arg_count, &frames,
                          error)) {
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OpCode::kSuperInvoke: {
        Constant method_name;
        if (!read_constant(active_chunk, frame.ip++, &method_name) ||
            !std::holds_alternative<std::string>(method_name)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid super invoke method operand",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid super invoke argument operand",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 2)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on super invoke",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t super_index = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
        const Value super_value = stack_[super_index];
        if (!super_value.is_object()) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "superclass must be a class"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass = std::dynamic_pointer_cast<ClassObject>(super_value.as_object());
        if (superclass == nullptr) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "superclass must be a class"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        stack_.erase(stack_.begin() + static_cast<std::ptrdiff_t>(super_index));
        if (!invoke_from_class(superclass, std::get<std::string>(method_name), arg_count, &frames,
                               error)) {
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OpCode::kCloseUpvalue: {
        if (stack_.empty()) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "stack underflow on close upvalue",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        close_upvalues(stack_.size() - 1);
        Value discarded;
        pop(&discarded);
        break;
      }
      case OpCode::kClass: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "class name must be string constant",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        auto klass = std::make_shared<ClassObject>(std::get<std::string>(c));
        push(Value(std::static_pointer_cast<RuntimeObject>(klass)));
        break;
      }
      case OpCode::kInherit: {
        Value superclass_value;
        Value subclass_value;
        if (!pop(&superclass_value) || !peek(&subclass_value)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for inheritance",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass =
            superclass_value.is_object()
                ? std::dynamic_pointer_cast<ClassObject>(superclass_value.as_object())
                : nullptr;
        const auto subclass =
            subclass_value.is_object() ? std::dynamic_pointer_cast<ClassObject>(subclass_value.as_object())
                                       : nullptr;
        if (superclass == nullptr || subclass == nullptr) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "superclass must be a class"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        subclass->superclass = superclass;
        break;
      }
      case OpCode::kMethod: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "method name must be string constant",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value method_value;
        Value class_value;
        if (!pop(&method_value) || !peek(&class_value)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for method binding",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto klass =
            class_value.is_object() ? std::dynamic_pointer_cast<ClassObject>(class_value.as_object()) : nullptr;
        const auto method = method_value.is_object()
                                ? std::dynamic_pointer_cast<ClosureObject>(method_value.as_object())
                                : nullptr;
        if (klass == nullptr || method == nullptr) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid method declaration",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        klass->methods.set(std::get<std::string>(c), Value(std::static_pointer_cast<RuntimeObject>(method)));
        break;
      }
      case OpCode::kGetProperty: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "property name must be string constant",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value receiver;
        if (!pop(&receiver)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "empty stack for property access",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const auto instance =
            receiver.is_object() ? std::dynamic_pointer_cast<InstanceObject>(receiver.as_object()) : nullptr;
        if (instance == nullptr) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "only instances have properties"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::string name = std::get<std::string>(c);
        Value value;
        if (instance->fields.get(name, &value)) {
          push(value);
          break;
        }
        if (!bind_method(instance->klass, name, receiver, error)) {
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OpCode::kSetProperty: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "property name must be string constant",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value value;
        Value receiver;
        if (!pop(&value) || !pop(&receiver)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for property set",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto instance =
            receiver.is_object() ? std::dynamic_pointer_cast<InstanceObject>(receiver.as_object()) : nullptr;
        if (instance == nullptr) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "only instances have fields"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        instance->fields.set(std::get<std::string>(c), value);
        push(value);
        break;
      }
      case OpCode::kGetSuper: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "super method name must be string constant",
                                                DiagnosticSpan{current_source_name_, 1}),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value superclass_value;
        Value receiver;
        if (!pop(&superclass_value) || !pop(&receiver)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for super access",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass =
            superclass_value.is_object()
                ? std::dynamic_pointer_cast<ClassObject>(superclass_value.as_object())
                : nullptr;
        if (superclass == nullptr) {
          set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "superclass must be a class"),
                                                  "runtime", "MS4003", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        if (!bind_method(superclass, std::get<std::string>(c), receiver, error)) {
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OpCode::kDefineGlobal:
      case OpCode::kGetGlobal:
      case OpCode::kSetGlobal:
      case OpCode::kImportModule: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
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
            set_single_diagnostic(ParseWithFallback(RuntimeError("MS4001", "undefined variable: " + name),
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
            set_single_diagnostic(ParseWithFallback(RuntimeError("MS4001", "undefined variable: " + name),
                                                    "runtime", "MS4001", current_source_name_),
                                  error);
            return InterpretResult::kRuntimeError;
          }
          break;
        }
        std::string module_error;
        auto module = modules_.load(name, *this, &module_error);
        if (!module) {
          set_single_diagnostic(ParseWithFallback(module_error, "module", "MS5004", current_source_name_),
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
        if (!read_constant(active_chunk, frame.ip++, &module_name) ||
            !read_constant(active_chunk, frame.ip++, &symbol_name) ||
            !read_constant(active_chunk, frame.ip++, &alias_name) ||
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
          set_single_diagnostic(ParseWithFallback(module_error, "module", "MS5004", current_source_name_),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value exported;
        if (!loaded->exports.get(symbol, &exported)) {
          set_single_diagnostic(
              ParseWithFallback("module error (MS5002): module '" + module +
                                    "' has no symbol '" + symbol + "'",
                                "module", "MS5002", current_source_name_),
              error);
          return InterpretResult::kRuntimeError;
        }
        define_global(alias, exported);
        break;
      }
      case OpCode::kJump:
      case OpCode::kJumpIfFalse:
      case OpCode::kLoop: {
        std::uint16_t offset = 0;
        if (!read_jump_offset(active_chunk, frame.ip, &offset)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid jump operand",
                              DiagnosticSpan{current_source_name_, 1}),
              error);
          return InterpretResult::kRuntimeError;
        }
        frame.ip += 2;
        if (op == OpCode::kJump) {
          frame.ip += offset;
          break;
        }
        if (op == OpCode::kJumpIfFalse) {
          Value condition;
          if (!peek(&condition)) {
            set_single_diagnostic(
                make_diagnostic("runtime", "MS4003", "empty stack for conditional jump",
                                DiagnosticSpan{current_source_name_, 1}),
                error);
            return InterpretResult::kRuntimeError;
          }
          if (is_falsey(condition)) {
            frame.ip += offset;
          }
          break;
        }
        frame.ip -= offset;
        break;
      }
      case OpCode::kReturn: {
        Value result = Value::nil();
        pop(&result);
        close_upvalues(frame.slot_base);
        stack_.resize(frame.slot_base);
        frames.pop_back();
        if (frames.empty()) {
          return InterpretResult::kOk;
        }
        push(result);
        break;
      }
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
    set_diagnostics(diagnostics, error);

    if (source_mode_ == SourceExecutionMode::kVmPreferredWithLegacyFallback &&
        ShouldFallbackAfterCompileError(diagnostics)) {
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

InterpretResult Vm::execute_module(const std::string& source, std::shared_ptr<Module> module,
                                   std::string* error) {
  auto prev = current_module_;
  current_module_ = std::move(module);
  const std::string previous_source_name = current_source_name_;
  const SourceExecutionMode previous_mode = source_mode_;
  source_mode_ = SourceExecutionMode::kVmPreferred;
  const std::string module_name = current_module_ != nullptr ? current_module_->name : "module.ms";
  const InterpretResult r = execute_source_named(source, module_name, error);
  source_mode_ = previous_mode;
  current_source_name_ = previous_source_name;
  current_module_ = prev;
  return r;
}

bool Vm::define_global(const std::string& name, Value value) {
  gc_.on_allocation(name.size() + value.to_string().size());
  if (gc_.should_collect()) {
    gc_.collect();
  }
  if (current_module_ != nullptr) {
    current_module_->exports.set(name, value);
    return true;
  }
  globals_.set(name, value);
  return true;
}

bool Vm::get_global(const std::string& name, Value* out) const {
  if (current_module_ != nullptr) {
    return current_module_->exports.get(name, out);
  }
  return globals_.get(name, out);
}

bool Vm::set_global(const std::string& name, Value value) {
  if (current_module_ != nullptr) {
    if (!current_module_->exports.contains(name)) {
      return false;
    }
    current_module_->exports.set(name, value);
    return true;
  }
  if (!globals_.contains(name)) {
    return false;
  }
  globals_.set(name, value);
  return true;
}

ModuleLoader& Vm::modules() noexcept { return modules_; }

GcController& Vm::gc() noexcept { return gc_; }

void Vm::set_source_execution_mode(const SourceExecutionMode mode) noexcept { source_mode_ = mode; }

SourceExecutionMode Vm::get_source_execution_mode() const noexcept { return source_mode_; }

SourceExecutionRoute Vm::last_source_execution_route() const noexcept { return last_source_route_; }

const std::vector<Diagnostic>& Vm::last_diagnostics() const noexcept { return last_diagnostics_; }

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

bool Vm::read_jump_offset(const Chunk& chunk, const std::size_t ip,
                          std::uint16_t* out) const noexcept {
  if (ip + 1 >= chunk.code().size()) {
    return false;
  }
  const std::uint16_t high = chunk.code()[ip];
  const std::uint16_t low = chunk.code()[ip + 1];
  if (out != nullptr) {
    *out = static_cast<std::uint16_t>((high << 8) | low);
  }
  return true;
}

bool Vm::is_falsey(const Value& value) const noexcept {
  if (value.is_nil()) {
    return true;
  }
  if (value.is_bool()) {
    return !value.as_bool();
  }
  return false;
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
  if (std::holds_alternative<std::string>(constant)) {
    return Value(std::get<std::string>(constant));
  }
  auto function = std::make_shared<FunctionObject>(std::get<std::shared_ptr<FunctionPrototype>>(constant));
  return Value(std::static_pointer_cast<RuntimeObject>(function));
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
bool Vm::call_closure(const std::shared_ptr<ClosureObject>& closure, const int arg_count,
                      std::vector<CallFrame>* frames, std::string* error) {
  if (closure == nullptr || closure->function == nullptr || closure->function->prototype == nullptr ||
      closure->function->prototype->chunk == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid callable object",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }

  const int expected = closure->function->prototype->arity;
  if (arg_count != expected) {
    set_single_diagnostic(
        ParseWithFallback(RuntimeError("MS4002", "expected " + std::to_string(expected) +
                                                    " arguments but got " + std::to_string(arg_count)),
                          "runtime", "MS4002", current_source_name_),
        error);
    return false;
  }

  if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid stack state for call",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }

  const std::size_t slot_base = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
  frames->push_back(CallFrame{closure, 0, slot_base});
  return true;
}

bool Vm::invoke_from_class(const std::shared_ptr<ClassObject>& klass, const std::string& name,
                           const int arg_count, std::vector<CallFrame>* frames,
                           std::string* error) {
  if (klass == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid class receiver",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }
  Value method;
  if (!klass->methods.get(name, &method)) {
    set_single_diagnostic(ParseWithFallback(RuntimeError("MS4004", "undefined property: " + name),
                                            "runtime", "MS4004", current_source_name_),
                          error);
    return false;
  }
  const auto closure =
      method.is_object() ? std::dynamic_pointer_cast<ClosureObject>(method.as_object()) : nullptr;
  if (closure == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "method must be callable closure",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }
  return call_closure(closure, arg_count, frames, error);
}

bool Vm::invoke_value(const Value& receiver, const std::string& name, const int arg_count,
                      std::vector<CallFrame>* frames, std::string* error) {
  const auto instance =
      receiver.is_object() ? std::dynamic_pointer_cast<InstanceObject>(receiver.as_object()) : nullptr;
  if (instance == nullptr) {
    set_single_diagnostic(ParseWithFallback(RuntimeError("MS4003", "only instances have methods"),
                                            "runtime", "MS4003", current_source_name_),
                          error);
    return false;
  }

  const std::size_t receiver_slot = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
  Value field;
  if (instance->fields.get(name, &field)) {
    stack_[receiver_slot] = field;
    return call_value_at(receiver_slot, arg_count, frames, error);
  }
  return invoke_from_class(instance->klass, name, arg_count, frames, error);
}

bool Vm::bind_method(const std::shared_ptr<ClassObject>& klass, const std::string& name,
                     const Value& receiver, std::string* error) {
  if (klass == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid class receiver",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }
  Value method;
  if (!klass->methods.get(name, &method)) {
    set_single_diagnostic(ParseWithFallback(RuntimeError("MS4004", "undefined property: " + name),
                                            "runtime", "MS4004", current_source_name_),
                          error);
    return false;
  }
  const auto closure =
      method.is_object() ? std::dynamic_pointer_cast<ClosureObject>(method.as_object()) : nullptr;
  if (closure == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "method must be callable closure",
                        DiagnosticSpan{current_source_name_, 1}),
        error);
    return false;
  }
  auto bound = std::make_shared<BoundMethodObject>(receiver, closure);
  push(Value(std::static_pointer_cast<RuntimeObject>(bound)));
  return true;
}

bool Vm::call_value_at(const std::size_t callee_index, const int arg_count,
                       std::vector<CallFrame>* frames, std::string* error) {
  if (callee_index >= stack_.size()) {
    set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid call target slot",
                                          DiagnosticSpan{current_source_name_, 1}),
                          error);
    return false;
  }

  const Value callee = stack_[callee_index];
  if (!callee.is_object()) {
    set_single_diagnostic(ParseWithFallback(RuntimeError("MS4005", "can only call functions and classes"),
                                            "runtime", "MS4005", current_source_name_),
                          error);
    return false;
  }

  const auto object = callee.as_object();
  if (const auto closure = std::dynamic_pointer_cast<ClosureObject>(object); closure != nullptr) {
    return call_closure(closure, arg_count, frames, error);
  }

  if (const auto klass = std::dynamic_pointer_cast<ClassObject>(object); klass != nullptr) {
    auto instance = std::make_shared<InstanceObject>(klass);
    stack_[callee_index] = Value(std::static_pointer_cast<RuntimeObject>(instance));

    Value init_method;
    if (!klass->methods.get("init", &init_method)) {
      if (arg_count != 0) {
        set_single_diagnostic(
            ParseWithFallback(RuntimeError("MS4002", "expected 0 arguments but got " +
                                                        std::to_string(arg_count)),
                              "runtime", "MS4002", current_source_name_),
            error);
        return false;
      }
      return true;
    }

    const auto init_closure = init_method.is_object()
                                  ? std::dynamic_pointer_cast<ClosureObject>(init_method.as_object())
                                  : nullptr;
    if (init_closure == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "initializer must be callable closure",
                          DiagnosticSpan{current_source_name_, 1}),
          error);
      return false;
    }
    return call_closure(init_closure, arg_count, frames, error);
  }

  if (const auto bound = std::dynamic_pointer_cast<BoundMethodObject>(object); bound != nullptr) {
    if (bound->method == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "invalid bound method target",
                          DiagnosticSpan{current_source_name_, 1}),
          error);
      return false;
    }
    stack_[callee_index] = bound->receiver;
    return call_closure(bound->method, arg_count, frames, error);
  }

  set_single_diagnostic(ParseWithFallback(RuntimeError("MS4005", "can only call functions and classes"),
                                          "runtime", "MS4005", current_source_name_),
                        error);
  return false;
}

std::shared_ptr<UpvalueObject> Vm::capture_upvalue(const std::size_t stack_index) {
  for (const auto& upvalue : open_upvalues_) {
    if (upvalue != nullptr && !upvalue->is_closed && upvalue->stack_index == stack_index) {
      return upvalue;
    }
  }
  auto created = std::make_shared<UpvalueObject>(stack_index);
  open_upvalues_.push_back(created);
  return created;
}

void Vm::close_upvalues(const std::size_t min_stack_index) {
  for (const auto& upvalue : open_upvalues_) {
    if (upvalue == nullptr || upvalue->is_closed || upvalue->stack_index < min_stack_index) {
      continue;
    }
    if (upvalue->stack_index < stack_.size()) {
      upvalue->closed = stack_[upvalue->stack_index];
    } else {
      upvalue->closed = Value::nil();
    }
    upvalue->is_closed = true;
  }
  open_upvalues_.erase(
      std::remove_if(open_upvalues_.begin(), open_upvalues_.end(),
                     [](const std::shared_ptr<UpvalueObject>& upvalue) {
                       return upvalue == nullptr || upvalue->is_closed;
                     }),
      open_upvalues_.end());
}

Value Vm::read_upvalue(const std::shared_ptr<UpvalueObject>& upvalue) const {
  if (upvalue == nullptr) {
    return Value::nil();
  }
  if (upvalue->is_closed) {
    return upvalue->closed;
  }
  if (upvalue->stack_index >= stack_.size()) {
    return Value::nil();
  }
  return stack_[upvalue->stack_index];
}

void Vm::write_upvalue(const std::shared_ptr<UpvalueObject>& upvalue, Value value) {
  if (upvalue == nullptr) {
    return;
  }
  if (upvalue->is_closed) {
    upvalue->closed = std::move(value);
    return;
  }
  if (upvalue->stack_index >= stack_.size()) {
    upvalue->closed = std::move(value);
    upvalue->is_closed = true;
    return;
  }
  stack_[upvalue->stack_index] = std::move(value);
}

}  // namespace ms
