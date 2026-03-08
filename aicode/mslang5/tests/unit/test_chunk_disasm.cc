#include "test_common.hh"

#include "bytecode/chunk.hh"
#include "bytecode/disasm.hh"

int RunChunkDisasmTests() {
  ms::Chunk chunk;
  const std::size_t c = chunk.AddConstant(42.0);
  chunk.WriteOp(ms::OpCode::kConstant, 1);
  chunk.Write(static_cast<std::uint8_t>(c), 1);
  chunk.WriteOp(ms::OpCode::kPrint, 1);
  chunk.WriteOp(ms::OpCode::kReturn, 1);

  const std::string text = ms::DisassembleChunk(chunk, "unit");
  Expect(text.find("OP_CONSTANT") != std::string::npos, "disasm should include constant");
  Expect(text.find("42") != std::string::npos, "disasm should include constant value");
  return 0;
}

