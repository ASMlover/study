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
  return "\"" + std::get<std::string>(constant) + "\"";
}

const char* OpName(const OpCode op) {
  switch (op) {
    case OpCode::kConstant:
      return "OP_CONSTANT";
    case OpCode::kAdd:
      return "OP_ADD";
    case OpCode::kSubtract:
      return "OP_SUBTRACT";
    case OpCode::kMultiply:
      return "OP_MULTIPLY";
    case OpCode::kDivide:
      return "OP_DIVIDE";
    case OpCode::kNegate:
      return "OP_NEGATE";
    case OpCode::kPrint:
      return "OP_PRINT";
    case OpCode::kPop:
      return "OP_POP";
    case OpCode::kDefineGlobal:
      return "OP_DEFINE_GLOBAL";
    case OpCode::kGetGlobal:
      return "OP_GET_GLOBAL";
    case OpCode::kSetGlobal:
      return "OP_SET_GLOBAL";
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

    switch (op) {
      case OpCode::kConstant:
      case OpCode::kDefineGlobal:
      case OpCode::kGetGlobal:
      case OpCode::kSetGlobal:
      case OpCode::kImportModule:
        if (offset + 1 < chunk.code().size()) {
          print_const_operand(offset + 1);
        }
        offset += 2;
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
