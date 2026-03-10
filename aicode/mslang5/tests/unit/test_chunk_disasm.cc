#include "test_common.hh"

#include "bytecode/chunk.hh"
#include "bytecode/disasm.hh"

int RunChunkDisasmTests() {
  ms::Chunk chunk;
  const std::size_t c = chunk.add_constant(42.0);
  chunk.write_op(ms::OpCode::kConstant, 1);
  chunk.write(static_cast<std::uint8_t>(c), 1);
  chunk.write_op(ms::OpCode::kPrint, 1);
  chunk.write_op(ms::OpCode::kReturn, 1);

  const std::string text = ms::disassemble_chunk(chunk, "unit");
  Expect(text.find("OP_CONSTANT") != std::string::npos, "disasm should include constant");
  Expect(text.find("42") != std::string::npos, "disasm should include constant value");
  return 0;
}

