#include "test_common.hh"

#include <cstdint>
#include <string>
#include <vector>

#include "bytecode/opcode.hh"
#include "frontend/compiler.hh"

namespace {

std::uint8_t Op(const ms::OpCode op) { return static_cast<std::uint8_t>(op); }

bool IsNumberConstant(const ms::Constant& constant, const double value) {
  return std::holds_alternative<double>(constant) && std::get<double>(constant) == value;
}

bool IsStringConstant(const ms::Constant& constant, const std::string& value) {
  return std::holds_alternative<std::string>(constant) && std::get<std::string>(constant) == value;
}

void ExpectCodeEquals(const ms::CompileResult& result, const std::vector<std::uint8_t>& expected,
                      const std::string& fixture_name) {
  Expect(result.chunk.code() == expected, fixture_name + " should preserve baseline bytecode sequence");
}

void ExpectNoCompileErrors(const ms::CompileResult& result, const std::string& fixture_name) {
  Expect(result.errors.empty(), fixture_name + " should compile without diagnostics");
}

}  // namespace

int RunLoweringPhaseTests() {
  {
    const std::string fixture_name = "arithmetic-global-print fixture";
    const ms::CompileResult result = ms::compile_to_chunk(
        "var x = 1 + 2;\n"
        "print x;\n");
    ExpectNoCompileErrors(result, fixture_name);

    ExpectCodeEquals(result,
                     {Op(ms::OpCode::kConstant), 0, Op(ms::OpCode::kConstant), 1,
                      Op(ms::OpCode::kAdd), Op(ms::OpCode::kDefineGlobal), 2,
                      Op(ms::OpCode::kGetGlobal), 3, Op(ms::OpCode::kPrint),
                      Op(ms::OpCode::kReturn)},
                     fixture_name);

    Expect(result.chunk.constants().size() == 4,
           fixture_name + " should preserve baseline constant pool width");
    Expect(IsNumberConstant(result.chunk.constants()[0], 1.0),
           fixture_name + " should keep first numeric constant");
    Expect(IsNumberConstant(result.chunk.constants()[1], 2.0),
           fixture_name + " should keep second numeric constant");
    Expect(IsStringConstant(result.chunk.constants()[2], "x"),
           fixture_name + " should keep first symbol constant");
    Expect(IsStringConstant(result.chunk.constants()[3], "x"),
           fixture_name + " should keep second symbol constant");
  }

  {
    const std::string fixture_name = "scoped-local-assignment fixture";
    const ms::CompileResult result = ms::compile_to_chunk(
        "{\n"
        "  var x = 1;\n"
        "  x = x + 2;\n"
        "  print x;\n"
        "}\n");
    ExpectNoCompileErrors(result, fixture_name);

    ExpectCodeEquals(result,
                     {Op(ms::OpCode::kConstant), 0, Op(ms::OpCode::kGetLocal), 0,
                      Op(ms::OpCode::kConstant), 1, Op(ms::OpCode::kAdd),
                      Op(ms::OpCode::kSetLocal), 0, Op(ms::OpCode::kGetLocal), 0,
                      Op(ms::OpCode::kPrint), Op(ms::OpCode::kPop), Op(ms::OpCode::kReturn)},
                     fixture_name);

    Expect(result.chunk.constants().size() == 2,
           fixture_name + " should preserve baseline constant pool width");
    Expect(IsNumberConstant(result.chunk.constants()[0], 1.0),
           fixture_name + " should keep first local numeric constant");
    Expect(IsNumberConstant(result.chunk.constants()[1], 2.0),
           fixture_name + " should keep second local numeric constant");
  }

  {
    const std::string fixture_name = "module-import fixture";
    const ms::CompileResult result = ms::compile_to_chunk(
        "import math;\n"
        "from pkg.mod import name as alias;\n");
    ExpectNoCompileErrors(result, fixture_name);

    ExpectCodeEquals(result,
                     {Op(ms::OpCode::kImportModule), 0, Op(ms::OpCode::kImportSymbol), 1, 2, 3,
                      Op(ms::OpCode::kReturn)},
                     fixture_name);

    Expect(result.chunk.constants().size() == 4,
           fixture_name + " should preserve baseline constant pool width");
    Expect(IsStringConstant(result.chunk.constants()[0], "math"),
           fixture_name + " should keep import-module constant");
    Expect(IsStringConstant(result.chunk.constants()[1], "pkg.mod"),
           fixture_name + " should keep from-module constant");
    Expect(IsStringConstant(result.chunk.constants()[2], "name"),
           fixture_name + " should keep imported symbol constant");
    Expect(IsStringConstant(result.chunk.constants()[3], "alias"),
           fixture_name + " should keep alias constant");
  }

  return 0;
}
