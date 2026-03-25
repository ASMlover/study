#include "runtime/vm.hh"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <memory>
#include <span>
#include <unordered_set>
#include <utility>

#include "bytecode/opcode.hh"
#include "frontend/compiler.hh"

namespace ms {

namespace {


std::string RuntimeError(const std::string& code, const std::string& message) {
  return "runtime error (" + code + "): " + message;
}

Diagnostic ParseWithFallback(const std::string& text, const std::string& phase,
                             const std::string& code, const std::string& file) {
  return parse_diagnostic_text(text, phase, code, file);
}

std::string NativeTypeName(const Value& value) {
  if (value.is_nil()) {
    return "nil";
  }
  if (value.is_bool()) {
    return "bool";
  }
  if (value.is_number()) {
    return "number";
  }
  if (value.is_string()) {
    return "string";
  }
  if (value.is_module()) {
    return "module";
  }
  if (!value.is_object()) {
    return "unknown";
  }

  RuntimeObject* object = value.as_object();
  if (object == nullptr) {
    return "object";
  }
  if (dynamic_cast<ClassObject*>(object) != nullptr) {
    return "class";
  }
  if (dynamic_cast<InstanceObject*>(object) != nullptr) {
    return "instance";
  }
  if (dynamic_cast<NativeFunctionObject*>(object) != nullptr) {
    return "native_function";
  }
  if (dynamic_cast<BoundMethodObject*>(object) != nullptr) {
    return "bound_method";
  }
  if (dynamic_cast<ClosureObject*>(object) != nullptr ||
      dynamic_cast<FunctionObject*>(object) != nullptr) {
    return "function";
  }
  if (dynamic_cast<UpvalueObject*>(object) != nullptr) {
    return "upvalue";
  }
  return "object";
}


}  // namespace

Vm::Vm() : out_(&std::cout), gc_(1024 * 64) { install_core_natives(); }

void Vm::set_output(std::ostream& out) noexcept { out_ = &out; }

std::ostream& Vm::output() const noexcept { return *out_; }

InterpretResult Vm::execute(const Chunk& chunk, std::string* error) {
  last_diagnostics_.clear();
  current_runtime_line_ = 1;
  stack_.clear();
  open_upvalues_.clear();
  gc_frame_roots_.clear();

  struct FrameRootsCleanup {
    std::vector<ClosureObject*>* roots = nullptr;
    ~FrameRootsCleanup() {
      if (roots != nullptr) {
        roots->clear();
      }
    }
  } frame_roots_cleanup{&gc_frame_roots_};

  auto script_proto = std::make_shared<FunctionPrototype>();
  script_proto->name = "<script>";
  script_proto->chunk = std::make_shared<Chunk>(chunk);
  auto script_fn = std::make_shared<FunctionObject>(std::move(script_proto));
  register_object_allocation(script_fn);
  auto script_closure = std::make_shared<ClosureObject>(script_fn.get());
  register_object_allocation(script_closure);

  std::vector<CallFrame> frames;
  frames.push_back(CallFrame{script_closure.get(), 0, 0});
  gc_frame_roots_.push_back(script_closure.get());

  while (!frames.empty()) {
    CallFrame& frame = frames.back();
    if (frame.closure == nullptr || frame.closure->function == nullptr ||
        frame.closure->function->prototype == nullptr ||
        frame.closure->function->prototype->chunk == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "invalid call frame",
                          current_runtime_span()),
          error);
      return InterpretResult::kRuntimeError;
    }

    const Chunk& active_chunk = *frame.closure->function->prototype->chunk;
    if (frame.ip >= active_chunk.code().size()) {
      return InterpretResult::kOk;
    }

    const auto instruction_ip = frame.ip;
    const auto op = static_cast<OpCode>(active_chunk.code()[frame.ip++]);
    current_runtime_line_ = line_for_instruction(active_chunk, instruction_ip);
    switch (op) {
      case OpCode::kConstant: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid constant index",
                                                current_runtime_span()),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const auto prototype = std::get<std::shared_ptr<FunctionPrototype>>(c);
        auto function = std::make_shared<FunctionObject>(prototype);
        register_object_allocation(function);
        auto closure = std::make_shared<ClosureObject>(function.get());
        register_object_allocation(closure);
        closure->upvalues.resize(static_cast<std::size_t>(prototype->upvalue_count));

        for (int i = 0; i < prototype->upvalue_count; ++i) {
          if (frame.ip + 1 >= active_chunk.code().size()) {
            set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid closure operand",
                                                  current_runtime_span()),
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
                                current_runtime_span()),
                error);
            return InterpretResult::kRuntimeError;
          }
          closure->upvalues[static_cast<std::size_t>(i)] =
              frame.closure->upvalues[static_cast<std::size_t>(index)];
        }

        push(Value(static_cast<RuntimeObject*>(closure.get())));
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
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "operands must be numbers"), "runtime", "MS4003"),
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
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "operands must be numbers"), "runtime", "MS4003"),
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
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "operand must be number"), "runtime", "MS4003"),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t slot = active_chunk.code()[frame.ip++];
        const std::size_t absolute_slot = frame.slot_base + slot;
        if (absolute_slot >= stack_.size()) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4001", "undefined local slot"), "runtime", "MS4001"),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t slot = active_chunk.code()[frame.ip++];
        if (slot >= frame.closure->upvalues.size() || frame.closure->upvalues[slot] == nullptr) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "upvalue slot out of range",
                                                current_runtime_span()),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on call",
                                                current_runtime_span()),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid invoke argument operand",
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on invoke",
                                                current_runtime_span()),
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
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        if (frame.ip >= active_chunk.code().size()) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid super invoke argument operand",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const int arg_count = active_chunk.code()[frame.ip++];
        if (stack_.size() < static_cast<std::size_t>(arg_count + 2)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "stack underflow on super invoke",
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::size_t super_index = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
        const Value super_value = stack_[super_index];
        if (!super_value.is_object()) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "superclass must be a class"), "runtime", "MS4003"),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass = dynamic_cast<ClassObject*>(super_value.as_object());
        if (superclass == nullptr) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "superclass must be a class"), "runtime", "MS4003"),
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
                              current_runtime_span()),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        auto klass = std::make_shared<ClassObject>(std::get<std::string>(c));
        register_object_allocation(klass);
        push(Value(static_cast<RuntimeObject*>(klass.get())));
        break;
      }
      case OpCode::kInherit: {
        Value superclass_value;
        Value subclass_value;
        if (!pop(&superclass_value) || !peek(&subclass_value)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for inheritance",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass =
            superclass_value.is_object()
                ? dynamic_cast<ClassObject*>(superclass_value.as_object())
                : nullptr;
        const auto subclass =
            subclass_value.is_object() ? dynamic_cast<ClassObject*>(subclass_value.as_object())
                                       : nullptr;
        if (superclass == nullptr || subclass == nullptr) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "superclass must be a class"), "runtime", "MS4003"),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value method_value;
        Value class_value;
        if (!pop(&method_value) || !peek(&class_value)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for method binding",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto klass =
            class_value.is_object() ? dynamic_cast<ClassObject*>(class_value.as_object()) : nullptr;
        const auto method = method_value.is_object()
                                ? dynamic_cast<ClosureObject*>(method_value.as_object())
                                : nullptr;
        if (klass == nullptr || method == nullptr) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid method declaration",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        klass->methods.set(std::get<std::string>(c), Value(static_cast<RuntimeObject*>(method)));
        break;
      }
      case OpCode::kGetProperty: {
        Constant c;
        if (!read_constant(active_chunk, frame.ip++, &c) || !std::holds_alternative<std::string>(c)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003",
                                                "property name must be string constant",
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value receiver;
        if (!pop(&receiver)) {
          set_single_diagnostic(make_diagnostic("runtime", "MS4003", "empty stack for property access",
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        const std::string name = std::get<std::string>(c);
        if (receiver.is_module()) {
          Value exported;
          if (!receiver.as_module()->exports.get(name, &exported)) {
            set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4004", "undefined property: " + name), "runtime", "MS4004"),
                                  error);
            return InterpretResult::kRuntimeError;
          }
          push(exported);
          break;
        }
        const auto instance =
            receiver.is_object() ? dynamic_cast<InstanceObject*>(receiver.as_object()) : nullptr;
        if (instance == nullptr) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "only instances have properties"), "runtime", "MS4003"),
                                error);
          return InterpretResult::kRuntimeError;
        }
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value value;
        Value receiver;
        if (!pop(&value) || !pop(&receiver)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for property set",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto instance =
            receiver.is_object() ? dynamic_cast<InstanceObject*>(receiver.as_object()) : nullptr;
        if (instance == nullptr) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "only instances have fields"), "runtime", "MS4003"),
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
                                                current_runtime_span()),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value superclass_value;
        Value receiver;
        if (!pop(&superclass_value) || !pop(&receiver)) {
          set_single_diagnostic(
              make_diagnostic("runtime", "MS4003", "invalid stack state for super access",
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const auto superclass =
            superclass_value.is_object()
                ? dynamic_cast<ClassObject*>(superclass_value.as_object())
                : nullptr;
        if (superclass == nullptr) {
          set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "superclass must be a class"), "runtime", "MS4003"),
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
                                                current_runtime_span()),
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
            set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4001", "undefined variable: " + name), "runtime", "MS4001"),
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
            set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4001", "undefined variable: " + name), "runtime", "MS4001"),
                                  error);
            return InterpretResult::kRuntimeError;
          }
          break;
        }
        std::string module_error;
        auto module = modules_.load(name, *this, &module_error);
        if (!module) {
          set_single_diagnostic(parse_diagnostic_with_current_span(module_error, "module", "MS5004"),
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
                              current_runtime_span()),
              error);
          return InterpretResult::kRuntimeError;
        }
        const std::string module = std::get<std::string>(module_name);
        const std::string symbol = std::get<std::string>(symbol_name);
        const std::string alias = std::get<std::string>(alias_name);

        std::string module_error;
        auto loaded = modules_.load(module, *this, &module_error);
        if (!loaded) {
          set_single_diagnostic(parse_diagnostic_with_current_span(module_error, "module", "MS5004"),
                                error);
          return InterpretResult::kRuntimeError;
        }
        Value exported;
        if (!loaded->exports.get(symbol, &exported)) {
          set_single_diagnostic(
              parse_diagnostic_with_current_span("module error (MS5002): module '" + module +
                                    "' has no symbol '" + symbol + "'", "module", "MS5002"),
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
                              current_runtime_span()),
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
                                current_runtime_span()),
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
        if (!gc_frame_roots_.empty()) {
          gc_frame_roots_.pop_back();
        }
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
  current_runtime_line_ = 1;
  last_diagnostics_.clear();
  last_source_route_ = SourceExecutionRoute::kVmPipeline;


  CompileResult compiled = compile_to_chunk(source);
  if (!compiled.errors.empty()) {
    std::vector<Diagnostic> diagnostics;
    diagnostics.reserve(compiled.errors.size());
    for (const auto& item : compiled.errors) {
      diagnostics.push_back(ParseWithFallback(item, "parse", "MS2001", source_name));
    }
    set_diagnostics(diagnostics, error);


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
  if (current_module_ != nullptr) {
    register_module_allocation(current_module_);
  }
  const std::string module_name = current_module_ != nullptr ? current_module_->name : "module.ms";
  const InterpretResult r = execute_source_named(source, module_name, error);
  current_source_name_ = previous_source_name;
  current_module_ = prev;
  return r;
}

bool Vm::define_global(const std::string& name, Value value) {
  if (current_module_ != nullptr) {
    current_module_->exports.set(name, value);
    maybe_collect_garbage();
    return true;
  }
  globals_.set(name, value);
  maybe_collect_garbage();
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

std::shared_ptr<Module> Vm::load_standard_module(const std::string& module_name,
                                                std::string* error) {
  auto module = std::make_shared<Module>();
  module->name = module_name;
  module->initializing = false;
  module->initialized = true;

  bool install_ok = false;
  if (module_name == "std.io") {
    install_ok = install_std_io_exports(module.get());
  } else if (module_name == "std.math") {
    install_ok = install_std_math_exports(module.get());
  } else if (module_name == "std.str") {
    install_ok = install_std_str_exports(module.get());
  } else if (module_name == "std.time") {
    install_ok = install_std_time_exports(module.get());
  } else if (module_name == "std.debug") {
    install_ok = install_std_debug_exports(module.get());
  } else {
    return nullptr;
  }

  if (!install_ok) {
    if (error != nullptr) {
      *error = RuntimeError("MS4003", "failed to initialize standard module: " + module_name);
    }
    return nullptr;
  }

  register_module_allocation(module);
  return module;
}


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

Value Vm::constant_to_value(const Constant& constant) {
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
  register_object_allocation(function);
  return Value(static_cast<RuntimeObject*>(function.get()));
}

std::string Vm::last_segment(const std::string& dotted) const {
  const auto pos = dotted.find_last_of('.');
  if (pos == std::string::npos) {
    return dotted;
  }
  return dotted.substr(pos + 1);
}
std::size_t Vm::line_for_instruction(const Chunk& chunk,
                                     const std::size_t instruction_ip) const noexcept {
  const auto& lines = chunk.lines();
  if (lines.empty()) {
    return 1;
  }
  if (instruction_ip >= lines.size()) {
    return lines.back();
  }
  return lines[instruction_ip];
}

DiagnosticSpan Vm::current_runtime_span() const {
  return DiagnosticSpan{current_source_name_, current_runtime_line_};
}

Diagnostic Vm::make_runtime_diagnostic(const std::string& code,
                                       const std::string& message) const {
  return make_diagnostic("runtime", code, message, current_runtime_span());
}

Diagnostic Vm::parse_diagnostic_with_current_span(const std::string& text,
                                                  const std::string& phase,
                                                  const std::string& code) const {
  Diagnostic diagnostic = ParseWithFallback(text, phase, code, current_source_name_);
  diagnostic.span.file = current_source_name_;
  diagnostic.span.line = current_runtime_line_;
  diagnostic.span.column = std::nullopt;
  diagnostic.span.length = std::nullopt;
  return diagnostic;
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
bool Vm::call_closure(ClosureObject* closure, const int arg_count,
                      std::vector<CallFrame>* frames, std::string* error) {
  if (closure == nullptr || closure->function == nullptr || closure->function->prototype == nullptr ||
      closure->function->prototype->chunk == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid callable object",
                        current_runtime_span()),
        error);
    return false;
  }

  const int expected = closure->function->prototype->arity;
  if (arg_count != expected) {
    set_single_diagnostic(
        parse_diagnostic_with_current_span(RuntimeError("MS4002", "expected " + std::to_string(expected) +
                                                    " arguments but got " + std::to_string(arg_count)), "runtime", "MS4002"),
        error);
    return false;
  }

  if (stack_.size() < static_cast<std::size_t>(arg_count + 1)) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid stack state for call",
                        current_runtime_span()),
        error);
    return false;
  }

  const std::size_t slot_base = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
  frames->push_back(CallFrame{closure, 0, slot_base});
  gc_frame_roots_.push_back(closure);
  return true;
}

bool Vm::invoke_from_class(ClassObject* klass, const std::string& name,
                           const int arg_count, std::vector<CallFrame>* frames,
                           std::string* error) {
  if (klass == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid class receiver",
                        current_runtime_span()),
        error);
    return false;
  }
  Value method;
  if (!klass->methods.get(name, &method)) {
    set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4004", "undefined property: " + name), "runtime", "MS4004"),
                          error);
    return false;
  }
  const auto closure =
      method.is_object() ? dynamic_cast<ClosureObject*>(method.as_object()) : nullptr;
  if (closure == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "method must be callable closure",
                        current_runtime_span()),
        error);
    return false;
  }
  return call_closure(closure, arg_count, frames, error);
}

bool Vm::invoke_value(const Value& receiver, const std::string& name, const int arg_count,
                      std::vector<CallFrame>* frames, std::string* error) {
  const std::size_t receiver_slot = stack_.size() - static_cast<std::size_t>(arg_count) - 1;
  if (receiver.is_module()) {
    auto module = receiver.as_module();
    Value exported;
    if (!module->exports.get(name, &exported)) {
      set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4004", "undefined property: " + name), "runtime", "MS4004"),
                            error);
      return false;
    }
    stack_[receiver_slot] = exported;
    return call_value_at(receiver_slot, arg_count, frames, error);
  }

  const auto instance =
      receiver.is_object() ? dynamic_cast<InstanceObject*>(receiver.as_object()) : nullptr;
  if (instance == nullptr) {
    set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", "only instances have methods"), "runtime", "MS4003"),
                          error);
    return false;
  }

  Value field;
  if (instance->fields.get(name, &field)) {
    stack_[receiver_slot] = field;
    return call_value_at(receiver_slot, arg_count, frames, error);
  }
  return invoke_from_class(instance->klass, name, arg_count, frames, error);
}

bool Vm::bind_method(ClassObject* klass, const std::string& name,
                     const Value& receiver, std::string* error) {
  if (klass == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "invalid class receiver",
                        current_runtime_span()),
        error);
    return false;
  }
  Value method;
  if (!klass->methods.get(name, &method)) {
    set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4004", "undefined property: " + name), "runtime", "MS4004"),
                          error);
    return false;
  }
  const auto closure =
      method.is_object() ? dynamic_cast<ClosureObject*>(method.as_object()) : nullptr;
  if (closure == nullptr) {
    set_single_diagnostic(
        make_diagnostic("runtime", "MS4003", "method must be callable closure",
                        current_runtime_span()),
        error);
    return false;
  }
  auto bound = std::make_shared<BoundMethodObject>(receiver, closure);
  register_object_allocation(bound);
  push(Value(static_cast<RuntimeObject*>(bound.get())));
  return true;
}

bool Vm::call_value_at(const std::size_t callee_index, const int arg_count,
                       std::vector<CallFrame>* frames, std::string* error) {
  if (callee_index >= stack_.size()) {
    set_single_diagnostic(make_diagnostic("runtime", "MS4003", "invalid call target slot",
                                          current_runtime_span()),
                          error);
    return false;
  }

  const Value callee = stack_[callee_index];
  if (!callee.is_object()) {
    set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4005", "can only call functions and classes"), "runtime", "MS4005"),
                          error);
    return false;
  }

  const auto object = callee.as_object();
  if (auto* closure = dynamic_cast<ClosureObject*>(object); closure != nullptr) {
    return call_closure(closure, arg_count, frames, error);
  }

  if (auto* native = dynamic_cast<NativeFunctionObject*>(object); native != nullptr) {
    if (native->arity != arg_count) {
      set_single_diagnostic(
          parse_diagnostic_with_current_span(RuntimeError("MS4002", "expected " + std::to_string(native->arity) +
                                                      " arguments but got " + std::to_string(arg_count)), "runtime", "MS4002"),
          error);
      return false;
    }
    Value result = Value::nil();
    const std::span<const Value> args(stack_.data() + callee_index + 1,
                                      static_cast<std::size_t>(arg_count));
    std::string native_error;
    if (!native->callable || !native->callable(*this, args, &result, &native_error)) {
      const std::string reason =
          native_error.empty() ? "native callable execution failed" : native_error;
      set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4003", reason), "runtime", "MS4003"),
                            error);
      return false;
    }
    stack_.resize(callee_index);
    push(result);
    return true;
  }

  if (auto* klass = dynamic_cast<ClassObject*>(object); klass != nullptr) {
    auto instance = std::make_shared<InstanceObject>(klass);
    register_object_allocation(instance);
    stack_[callee_index] = Value(static_cast<RuntimeObject*>(instance.get()));

    Value init_method;
    if (!klass->methods.get("init", &init_method)) {
      if (arg_count != 0) {
        set_single_diagnostic(
            parse_diagnostic_with_current_span(RuntimeError("MS4002", "expected 0 arguments but got " +
                                                        std::to_string(arg_count)), "runtime", "MS4002"),
            error);
        return false;
      }
      return true;
    }

    ClosureObject* init_closure = init_method.is_object()
                                  ? dynamic_cast<ClosureObject*>(init_method.as_object())
                                  : nullptr;
    if (init_closure == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "initializer must be callable closure",
                          current_runtime_span()),
          error);
      return false;
    }
    return call_closure(init_closure, arg_count, frames, error);
  }

  if (auto* bound = dynamic_cast<BoundMethodObject*>(object); bound != nullptr) {
    if (bound->method == nullptr) {
      set_single_diagnostic(
          make_diagnostic("runtime", "MS4003", "invalid bound method target",
                          current_runtime_span()),
          error);
      return false;
    }
    stack_[callee_index] = bound->receiver;
    return call_closure(bound->method, arg_count, frames, error);
  }

  set_single_diagnostic(parse_diagnostic_with_current_span(RuntimeError("MS4005", "can only call functions and classes"), "runtime", "MS4005"),
                        error);
  return false;
}

UpvalueObject* Vm::capture_upvalue(const std::size_t stack_index) {
  for (const auto& upvalue : open_upvalues_) {
    if (upvalue != nullptr && !upvalue->is_closed && upvalue->stack_index == stack_index) {
      return upvalue;
    }
  }
  auto created = std::make_shared<UpvalueObject>(stack_index);
  register_object_allocation(created);
  open_upvalues_.push_back(created.get());
  return created.get();
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
                     [](const UpvalueObject* upvalue) {
                       return upvalue == nullptr || upvalue->is_closed;
                     }),
      open_upvalues_.end());
}

Value Vm::read_upvalue(const UpvalueObject* upvalue) const {
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

void Vm::write_upvalue(UpvalueObject* upvalue, Value value) {
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


void Vm::maybe_collect_garbage() {
  if (!gc_.should_collect()) {
    return;
  }
  gc_.collect([this](GcController& gc) { trace_gc_roots(gc); });
  prune_untracked_owned_objects();
}

void Vm::prune_untracked_owned_objects() {
  gc_owned_objects_.erase(
      std::remove_if(gc_owned_objects_.begin(), gc_owned_objects_.end(),
                     [](const std::shared_ptr<RuntimeObject>& object) {
                       return object == nullptr || !object->gc_header().tracked;
                     }),
      gc_owned_objects_.end());
}

void Vm::register_object_allocation(const std::shared_ptr<RuntimeObject>& object) {
  if (object == nullptr) {
    return;
  }
  maybe_collect_garbage();
  gc_owned_objects_.push_back(object);
  gc_.register_object(object.get(), estimate_object_bytes(object.get()));
}

void Vm::register_module_allocation(const std::shared_ptr<Module>& module) {
  if (module == nullptr) {
    return;
  }
  maybe_collect_garbage();
  gc_.register_allocation(module.get(), estimate_module_bytes(module));
}

RuntimeObject* Vm::create_native_object(const std::string& name, const int arity,
                                      NativeCallable callable) {
  if (name.empty() || callable == nullptr) {
    return nullptr;
  }
  auto native = std::make_unique<NativeFunctionObject>(name, arity, std::move(callable));
  RuntimeObject* native_ptr = native.get();
  native_owned_objects_.push_back(std::move(native));
  return native_ptr;
}

bool Vm::register_native(const std::string& name, const int arity, NativeCallable callable) {
  RuntimeObject* native_ptr = create_native_object(name, arity, std::move(callable));
  if (native_ptr == nullptr) {
    return false;
  }
  return globals_.set(name, Value(native_ptr));
}

bool Vm::install_std_io_exports(Module* module) {
  if (module == nullptr) {
    return false;
  }
  RuntimeObject* print_fn = create_native_object(
      "std.io.print", 1,
      [](Vm& vm, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.io.print expects exactly one argument";
          }
          return false;
        }
        vm.output() << args[0].to_string();
        if (out != nullptr) {
          *out = Value::nil();
        }
        return true;
      });
  RuntimeObject* println_fn = create_native_object(
      "std.io.println", 1,
      [](Vm& vm, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.io.println expects exactly one argument";
          }
          return false;
        }
        vm.output() << args[0].to_string() << '\n';
        if (out != nullptr) {
          *out = Value::nil();
        }
        return true;
      });
  if (print_fn == nullptr || println_fn == nullptr) {
    return false;
  }
  return module->exports.set("print", Value(print_fn)) &&
         module->exports.set("println", Value(println_fn));
}

bool Vm::install_std_math_exports(Module* module) {
  if (module == nullptr) {
    return false;
  }

  const auto expect_number = [](const Value& value, const std::string& fn_name,
                                const std::size_t index, std::string* native_error) -> bool {
    if (value.is_number()) {
      return true;
    }
    if (native_error != nullptr) {
      *native_error = fn_name + " argument " + std::to_string(index + 1) + " must be number";
    }
    return false;
  };

  RuntimeObject* abs_fn = create_native_object(
      "std.math.abs", 1,
      [expect_number](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.math.abs expects exactly one argument";
          }
          return false;
        }
        if (!expect_number(args[0], "std.math.abs", 0, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(std::fabs(args[0].as_number()));
        }
        return true;
      });
  RuntimeObject* min_fn = create_native_object(
      "std.math.min", 2,
      [expect_number](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 2) {
          if (native_error != nullptr) {
            *native_error = "std.math.min expects exactly two arguments";
          }
          return false;
        }
        if (!expect_number(args[0], "std.math.min", 0, native_error) ||
            !expect_number(args[1], "std.math.min", 1, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(std::min(args[0].as_number(), args[1].as_number()));
        }
        return true;
      });
  RuntimeObject* max_fn = create_native_object(
      "std.math.max", 2,
      [expect_number](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 2) {
          if (native_error != nullptr) {
            *native_error = "std.math.max expects exactly two arguments";
          }
          return false;
        }
        if (!expect_number(args[0], "std.math.max", 0, native_error) ||
            !expect_number(args[1], "std.math.max", 1, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(std::max(args[0].as_number(), args[1].as_number()));
        }
        return true;
      });
  RuntimeObject* floor_fn = create_native_object(
      "std.math.floor", 1,
      [expect_number](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.math.floor expects exactly one argument";
          }
          return false;
        }
        if (!expect_number(args[0], "std.math.floor", 0, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(std::floor(args[0].as_number()));
        }
        return true;
      });
  RuntimeObject* ceil_fn = create_native_object(
      "std.math.ceil", 1,
      [expect_number](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.math.ceil expects exactly one argument";
          }
          return false;
        }
        if (!expect_number(args[0], "std.math.ceil", 0, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(std::ceil(args[0].as_number()));
        }
        return true;
      });

  if (abs_fn == nullptr || min_fn == nullptr || max_fn == nullptr || floor_fn == nullptr ||
      ceil_fn == nullptr) {
    return false;
  }
  return module->exports.set("abs", Value(abs_fn)) && module->exports.set("min", Value(min_fn)) &&
         module->exports.set("max", Value(max_fn)) &&
         module->exports.set("floor", Value(floor_fn)) &&
         module->exports.set("ceil", Value(ceil_fn));
}

bool Vm::install_std_str_exports(Module* module) {
  if (module == nullptr) {
    return false;
  }

  const auto expect_string = [](const Value& value, const std::string& fn_name,
                                const std::size_t index, std::string* native_error) -> bool {
    if (value.is_string()) {
      return true;
    }
    if (native_error != nullptr) {
      *native_error = fn_name + " argument " + std::to_string(index + 1) + " must be string";
    }
    return false;
  };

  const auto expect_non_negative_integer = [](const Value& value, const std::string& fn_name,
                                              const std::size_t index,
                                              std::string* native_error) -> bool {
    if (!value.is_number()) {
      if (native_error != nullptr) {
        *native_error =
            fn_name + " argument " + std::to_string(index + 1) + " must be non-negative integer";
      }
      return false;
    }
    const double number = value.as_number();
    if (!std::isfinite(number) || number < 0 || std::floor(number) != number) {
      if (native_error != nullptr) {
        *native_error =
            fn_name + " argument " + std::to_string(index + 1) + " must be non-negative integer";
      }
      return false;
    }
    return true;
  };

  RuntimeObject* len_fn = create_native_object(
      "std.str.len", 1,
      [expect_string](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.str.len expects exactly one argument";
          }
          return false;
        }
        if (!expect_string(args[0], "std.str.len", 0, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(static_cast<double>(args[0].as_string().size()));
        }
        return true;
      });

  RuntimeObject* substr_fn = create_native_object(
      "std.str.substr", 3,
      [expect_string, expect_non_negative_integer](Vm&, std::span<const Value> args, Value* out,
                                                   std::string* native_error) {
        if (args.size() != 3) {
          if (native_error != nullptr) {
            *native_error = "std.str.substr expects exactly three arguments";
          }
          return false;
        }
        if (!expect_string(args[0], "std.str.substr", 0, native_error) ||
            !expect_non_negative_integer(args[1], "std.str.substr", 1, native_error) ||
            !expect_non_negative_integer(args[2], "std.str.substr", 2, native_error)) {
          return false;
        }

        const std::string& source = args[0].as_string();
        std::size_t start = static_cast<std::size_t>(args[1].as_number());
        const std::size_t length = static_cast<std::size_t>(args[2].as_number());
        if (start > source.size()) {
          start = source.size();
        }
        const std::size_t count = std::min(length, source.size() - start);

        if (out != nullptr) {
          *out = Value(source.substr(start, count));
        }
        return true;
      });

  RuntimeObject* contains_fn = create_native_object(
      "std.str.contains", 2,
      [expect_string](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 2) {
          if (native_error != nullptr) {
            *native_error = "std.str.contains expects exactly two arguments";
          }
          return false;
        }
        if (!expect_string(args[0], "std.str.contains", 0, native_error) ||
            !expect_string(args[1], "std.str.contains", 1, native_error)) {
          return false;
        }
        if (out != nullptr) {
          *out = Value(args[0].as_string().find(args[1].as_string()) != std::string::npos);
        }
        return true;
      });

  if (len_fn == nullptr || substr_fn == nullptr || contains_fn == nullptr) {
    return false;
  }
  return module->exports.set("len", Value(len_fn)) &&
         module->exports.set("substr", Value(substr_fn)) &&
         module->exports.set("contains", Value(contains_fn));
}

bool Vm::install_std_time_exports(Module* module) {
  if (module == nullptr) {
    return false;
  }

  RuntimeObject* now_unix_ms_fn = create_native_object(
      "std.time.now_unix_ms", 0,
      [](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (!args.empty()) {
          if (native_error != nullptr) {
            *native_error = "std.time.now_unix_ms expects no arguments";
          }
          return false;
        }
        const auto now = std::chrono::system_clock::now();
        const auto millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        if (out != nullptr) {
          *out = Value(static_cast<double>(millis));
        }
        return true;
      });

  if (now_unix_ms_fn == nullptr) {
    return false;
  }
  return module->exports.set("now_unix_ms", Value(now_unix_ms_fn));
}

bool Vm::install_std_debug_exports(Module* module) {
  if (module == nullptr) {
    return false;
  }

  RuntimeObject* typeof_fn = create_native_object(
      "std.debug.typeof", 1,
      [](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
        if (args.size() != 1) {
          if (native_error != nullptr) {
            *native_error = "std.debug.typeof expects exactly one argument";
          }
          return false;
        }
        if (out != nullptr) {
          *out = Value(NativeTypeName(args[0]));
        }
        return true;
      });

  if (typeof_fn == nullptr) {
    return false;
  }
  return module->exports.set("typeof", Value(typeof_fn));
}

void Vm::install_core_natives() {
  register_native("native_clock", 0,
                  [](Vm&, std::span<const Value>, Value* out, std::string*) {
                    const auto now = std::chrono::system_clock::now();
                    const auto millis =
                        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
                            .count();
                    if (out != nullptr) {
                      *out = Value(static_cast<double>(millis));
                    }
                    return true;
                  });

  register_native("native_require_number", 1,
                  [](Vm&, std::span<const Value> args, Value* out, std::string* native_error) {
                    if (args.size() != 1) {
                      if (native_error != nullptr) {
                        *native_error = "expected one argument";
                      }
                      return false;
                    }
                    if (!args[0].is_number()) {
                      if (native_error != nullptr) {
                        *native_error = "argument 1 must be number";
                      }
                      return false;
                    }
                    if (out != nullptr) {
                      *out = args[0];
                    }
                    return true;
                  });
}

std::size_t Vm::estimate_object_bytes(const RuntimeObject* object) const {
  if (object == nullptr) {
    return 0;
  }
  std::size_t bytes = sizeof(RuntimeObject);
  if (const auto* string_obj = dynamic_cast<const StringObject*>(object); string_obj != nullptr) {
    bytes += sizeof(StringObject) + string_obj->value.capacity();
    return bytes;
  }
  if (const auto* function_obj = dynamic_cast<const FunctionObject*>(object);
      function_obj != nullptr) {
    bytes += sizeof(FunctionObject);
    if (function_obj->prototype != nullptr) {
      bytes += sizeof(FunctionPrototype);
      if (function_obj->prototype->chunk != nullptr) {
        const auto& chunk = *function_obj->prototype->chunk;
        bytes += sizeof(Chunk);
        bytes += chunk.code().size() * sizeof(std::uint8_t);
        bytes += chunk.lines().size() * sizeof(std::size_t);
        bytes += chunk.constants().size() * sizeof(Constant);
      }
    }
    return bytes;
  }
  if (const auto* upvalue_obj = dynamic_cast<const UpvalueObject*>(object); upvalue_obj != nullptr) {
    bytes += sizeof(UpvalueObject);
    return bytes;
  }
  if (const auto* closure_obj = dynamic_cast<const ClosureObject*>(object); closure_obj != nullptr) {
    bytes += sizeof(ClosureObject);
    bytes += closure_obj->upvalues.capacity() * sizeof(UpvalueObject*);
    return bytes;
  }
  if (const auto* class_obj = dynamic_cast<const ClassObject*>(object); class_obj != nullptr) {
    bytes += sizeof(ClassObject);
    bytes += class_obj->name.capacity();
    bytes += class_obj->methods.size() * (sizeof(std::string) + sizeof(Value));
    return bytes;
  }
  if (const auto* instance_obj = dynamic_cast<const InstanceObject*>(object);
      instance_obj != nullptr) {
    bytes += sizeof(InstanceObject);
    bytes += instance_obj->fields.size() * (sizeof(std::string) + sizeof(Value));
    return bytes;
  }
  if (const auto* bound_obj = dynamic_cast<const BoundMethodObject*>(object);
      bound_obj != nullptr) {
    bytes += sizeof(BoundMethodObject);
    return bytes;
  }
  if (const auto* native_obj = dynamic_cast<const NativeFunctionObject*>(object);
      native_obj != nullptr) {
    bytes += sizeof(NativeFunctionObject);
    bytes += native_obj->name.capacity();
    return bytes;
  }
  return bytes;
}

std::size_t Vm::estimate_module_bytes(const std::shared_ptr<Module>& module) const {
  if (module == nullptr) {
    return 0;
  }
  return sizeof(Module) + module->name.capacity() +
         module->exports.size() * (sizeof(std::string) + sizeof(Value));
}

void Vm::trace_gc_roots(GcController& gc) const {
  std::unordered_set<const void*> seen_modules;
  std::unordered_set<const void*> seen_objects;
#ifndef NDEBUG
  std::vector<const void*> cached_module_keys;
#endif

  for (const auto& value : stack_) {
    trace_gc_value(value, gc, &seen_modules, &seen_objects);
  }
  for (const auto& upvalue : open_upvalues_) {
    trace_gc_object(upvalue, gc, &seen_modules, &seen_objects);
  }
  for (auto* closure : gc_frame_roots_) {
    trace_gc_object(closure, gc, &seen_modules, &seen_objects);
  }
  trace_gc_table(globals_, gc, &seen_modules, &seen_objects);

  if (current_module_ != nullptr) {
#ifndef NDEBUG
    assert(gc.is_registered_allocation(current_module_.get()) &&
           "current module root must be registered before tracing");
#endif
    trace_gc_module(current_module_, gc, &seen_modules, &seen_objects);
  }
  modules_.for_each_cached_module([this, &gc, &seen_modules, &seen_objects
#ifndef NDEBUG
                                   , &cached_module_keys
#endif
                                   ](const std::shared_ptr<Module>& module) {
#ifndef NDEBUG
    assert(module != nullptr && "module cache must not contain null module entries");
    if (module != nullptr) {
      assert(gc.is_registered_allocation(module.get()) &&
             "module cache entry must be registered before tracing");
      cached_module_keys.push_back(module.get());
    }
#endif
    trace_gc_module(module, gc, &seen_modules, &seen_objects);
  });

#ifndef NDEBUG
  if (current_module_ != nullptr) {
    assert(gc.is_allocation_marked(current_module_.get()) &&
           "current module allocation should be marked by root tracing");
  }
  for (const void* key : cached_module_keys) {
    assert(gc.is_allocation_marked(key) &&
           "module cache allocation should be marked by root tracing");
  }
  for (const auto& object : gc_owned_objects_) {
    assert(object != nullptr && "vm ownership container should not keep null entries");
    if (object != nullptr) {
      assert(gc.is_registered_object(object.get()) &&
             "vm-owned object should remain GC-registered");
    }
  }
#endif
}

void Vm::trace_gc_table(const Table& table, GcController& gc,
                        std::unordered_set<const void*>* seen_modules,
                        std::unordered_set<const void*>* seen_objects) const {
  for (const auto& [_, value] : table.data()) {
    trace_gc_value(value, gc, seen_modules, seen_objects);
  }
}

void Vm::trace_gc_value(const Value& value, GcController& gc,
                        std::unordered_set<const void*>* seen_modules,
                        std::unordered_set<const void*>* seen_objects) const {
  if (value.is_module()) {
    trace_gc_module(value.as_module(), gc, seen_modules, seen_objects);
    return;
  }
  if (value.is_object()) {
    trace_gc_object(value.as_object(), gc, seen_modules, seen_objects);
  }
}

void Vm::trace_gc_module(const std::shared_ptr<Module>& module, GcController& gc,
                         std::unordered_set<const void*>* seen_modules,
                         std::unordered_set<const void*>* seen_objects) const {
  if (module == nullptr) {
    return;
  }
  const void* key = module.get();
  if (seen_modules != nullptr && !seen_modules->insert(key).second) {
    return;
  }
  gc.mark_allocation(key);
  trace_gc_table(module->exports, gc, seen_modules, seen_objects);
}

void Vm::trace_gc_object(RuntimeObject* object, GcController& gc,
                         std::unordered_set<const void*>* seen_modules,
                         std::unordered_set<const void*>* seen_objects) const {
  if (object == nullptr) {
    return;
  }
  const void* key = object;
  if (seen_objects != nullptr && !seen_objects->insert(key).second) {
    return;
  }
  gc.mark_object(object);

  if (auto* closure_obj = dynamic_cast<ClosureObject*>(object);
      closure_obj != nullptr) {
    trace_gc_object(closure_obj->function, gc, seen_modules, seen_objects);
    for (const auto& upvalue : closure_obj->upvalues) {
      trace_gc_object(upvalue, gc, seen_modules, seen_objects);
    }
    return;
  }
  if (auto* upvalue_obj = dynamic_cast<UpvalueObject*>(object);
      upvalue_obj != nullptr) {
    trace_gc_value(upvalue_obj->closed, gc, seen_modules, seen_objects);
    return;
  }
  if (const auto* class_obj = dynamic_cast<const ClassObject*>(object); class_obj != nullptr) {
    trace_gc_object(class_obj->superclass, gc, seen_modules, seen_objects);
    trace_gc_table(class_obj->methods, gc, seen_modules, seen_objects);
    return;
  }
  if (const auto* instance_obj = dynamic_cast<const InstanceObject*>(object);
      instance_obj != nullptr) {
    trace_gc_object(instance_obj->klass, gc, seen_modules, seen_objects);
    trace_gc_table(instance_obj->fields, gc, seen_modules, seen_objects);
    return;
  }
  if (const auto* bound_obj = dynamic_cast<const BoundMethodObject*>(object);
      bound_obj != nullptr) {
    trace_gc_value(bound_obj->receiver, gc, seen_modules, seen_objects);
    trace_gc_object(bound_obj->method, gc, seen_modules, seen_objects);
  }
}
}  // namespace ms


