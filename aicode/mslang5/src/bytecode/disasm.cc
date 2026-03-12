#include "bytecode/disasm.hh"

#include <iomanip>
#include <sstream>

namespace ms {

namespace {

std::string ConstantToString(const Constant& constant) {
  if (std::holds_alternative<std::monostate>(constant)) {
    return "nil";
  }
  if (std::holds_alternative<bool>(constant)) {
    return std::get<bool>(constant) ? "true" : "false";
  }
  if (std::holds_alternative<double>(constant)) {
    std::ostringstream out;
    out << std::get<double>(constant);
    return out.str();
  }
  if (std::holds_alternative<std::string>(constant)) {
    return "\"" + std::get<std::string>(constant) + "\"";
  }
  const auto prototype = std::get<std::shared_ptr<FunctionPrototype>>(constant);
  if (prototype == nullptr) {
    return "<fn:null>";
  }
  return "<fn " + prototype->name + ">";
}

const char* OpName(const OpCode op) {
  switch (op) {
    case OpCode::kConstant:
      return "OP_CONSTANT";
    case OpCode::kClosure:
      return "OP_CLOSURE";
    case OpCode::kEqual:
      return "OP_EQUAL";
    case OpCode::kGreater:
      return "OP_GREATER";
    case OpCode::kLess:
      return "OP_LESS";
    case OpCode::kAdd:
      return "OP_ADD";
    case OpCode::kSubtract:
      return "OP_SUBTRACT";
    case OpCode::kMultiply:
      return "OP_MULTIPLY";
    case OpCode::kDivide:
      return "OP_DIVIDE";
    case OpCode::kNot:
      return "OP_NOT";
    case OpCode::kNegate:
      return "OP_NEGATE";
    case OpCode::kPrint:
      return "OP_PRINT";
    case OpCode::kPop:
      return "OP_POP";
    case OpCode::kGetLocal:
      return "OP_GET_LOCAL";
    case OpCode::kSetLocal:
      return "OP_SET_LOCAL";
    case OpCode::kGetUpvalue:
      return "OP_GET_UPVALUE";
    case OpCode::kSetUpvalue:
      return "OP_SET_UPVALUE";
    case OpCode::kCall:
      return "OP_CALL";
    case OpCode::kInvoke:
      return "OP_INVOKE";
    case OpCode::kSuperInvoke:
      return "OP_SUPER_INVOKE";
    case OpCode::kCloseUpvalue:
      return "OP_CLOSE_UPVALUE";
    case OpCode::kClass:
      return "OP_CLASS";
    case OpCode::kInherit:
      return "OP_INHERIT";
    case OpCode::kMethod:
      return "OP_METHOD";
    case OpCode::kGetProperty:
      return "OP_GET_PROPERTY";
    case OpCode::kSetProperty:
      return "OP_SET_PROPERTY";
    case OpCode::kGetSuper:
      return "OP_GET_SUPER";
    case OpCode::kDefineGlobal:
      return "OP_DEFINE_GLOBAL";
    case OpCode::kGetGlobal:
      return "OP_GET_GLOBAL";
    case OpCode::kSetGlobal:
      return "OP_SET_GLOBAL";
    case OpCode::kJump:
      return "OP_JUMP";
    case OpCode::kJumpIfFalse:
      return "OP_JUMP_IF_FALSE";
    case OpCode::kLoop:
      return "OP_LOOP";
    case OpCode::kImportModule:
      return "OP_IMPORT_MODULE";
    case OpCode::kImportSymbol:
      return "OP_IMPORT_SYMBOL";
    case OpCode::kReturn:
      return "OP_RETURN";
  }
  return "OP_UNKNOWN";
}

}  // namespace

std::string disassemble_chunk(const Chunk& chunk, const std::string& name) {
  std::ostringstream out;
  out << "== " << name << " ==\n";

  std::size_t offset = 0;
  while (offset < chunk.code().size()) {
    const auto op = static_cast<OpCode>(chunk.code()[offset]);
    out << std::setw(4) << offset << " " << OpName(op);

    auto print_const_operand = [&](const std::size_t i) {
      const std::size_t idx = chunk.code()[i];
      out << " " << idx;
      if (idx < chunk.constants().size()) {
        out << " (" << ConstantToString(chunk.constants()[idx]) << ")";
      }
    };
    auto print_raw_operand = [&](const std::size_t i) { out << " " << chunk.code()[i]; };

    switch (op) {
      case OpCode::kConstant:
      case OpCode::kDefineGlobal:
      case OpCode::kGetGlobal:
      case OpCode::kSetGlobal:
      case OpCode::kImportModule:
      case OpCode::kClass:
      case OpCode::kMethod:
      case OpCode::kGetProperty:
      case OpCode::kSetProperty:
      case OpCode::kGetSuper:
        if (offset + 1 < chunk.code().size()) {
          print_const_operand(offset + 1);
        }
        offset += 2;
        break;
      case OpCode::kCall:
        if (offset + 1 < chunk.code().size()) {
          print_raw_operand(offset + 1);
        }
        offset += 2;
        break;
      case OpCode::kInvoke:
      case OpCode::kSuperInvoke:
        if (offset + 2 < chunk.code().size()) {
          print_const_operand(offset + 1);
          print_raw_operand(offset + 2);
        }
        offset += 3;
        break;
      case OpCode::kClosure:
        if (offset + 1 < chunk.code().size()) {
          const std::size_t const_index = chunk.code()[offset + 1];
          print_const_operand(offset + 1);
          std::size_t consumed = 2;
          if (const_index < chunk.constants().size() &&
              std::holds_alternative<std::shared_ptr<FunctionPrototype>>(chunk.constants()[const_index])) {
            const auto prototype =
                std::get<std::shared_ptr<FunctionPrototype>>(chunk.constants()[const_index]);
            if (prototype != nullptr) {
              for (int i = 0; i < prototype->upvalue_count; ++i) {
                if (offset + consumed + 1 >= chunk.code().size()) {
                  break;
                }
                out << " [" << static_cast<int>(chunk.code()[offset + consumed]) << ","
                    << static_cast<int>(chunk.code()[offset + consumed + 1]) << "]";
                consumed += 2;
              }
            }
          }
          offset += consumed;
          break;
        }
        offset += 1;
        break;
      case OpCode::kGetLocal:
      case OpCode::kSetLocal:
      case OpCode::kGetUpvalue:
      case OpCode::kSetUpvalue:
        if (offset + 1 < chunk.code().size()) {
          print_raw_operand(offset + 1);
        }
        offset += 2;
        break;
      case OpCode::kJump:
      case OpCode::kJumpIfFalse:
      case OpCode::kLoop:
        if (offset + 2 < chunk.code().size()) {
          const std::size_t jump = (static_cast<std::size_t>(chunk.code()[offset + 1]) << 8) |
                                   static_cast<std::size_t>(chunk.code()[offset + 2]);
          out << " " << jump;
        }
        offset += 3;
        break;
      case OpCode::kImportSymbol:
        if (offset + 3 < chunk.code().size()) {
          print_const_operand(offset + 1);
          print_const_operand(offset + 2);
          print_const_operand(offset + 3);
        }
        offset += 4;
        break;
      default:
        offset += 1;
        break;
    }
    out << "\n";
  }
  return out.str();
}

}  // namespace ms

