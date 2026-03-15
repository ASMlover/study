// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <format>
#include "Compiler.hh"
#include "VM.hh"
#include "Memory.hh"
#include "Debug.hh"

namespace ms {

enum class Precedence : int {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_TERNARY,     // ?:
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_BIT_OR,      // |
  PREC_BIT_XOR,     // ^
  PREC_BIT_AND,     // &
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_SHIFT,       // << >>
  PREC_TERM,        // + -
  PREC_FACTOR,      // * / %
  PREC_UNARY,       // ! - ~
  PREC_CALL,        // . ()
  PREC_PRIMARY,
};

using ParseFn = void (Compiler::*)(bool can_assign);

struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct Local {
  Token name{};
  int depth{-1};
  bool is_captured{false};
};

struct Upvalue {
  u8_t index{0};
  bool is_local{false};
};

struct LoopContext {
  sz_t continue_target{0};
  int break_depth{0};
  int continue_depth{0};
  std::vector<sz_t> break_jumps;
  LoopContext* enclosing{nullptr};
};

struct ClassCompiler {
  ClassCompiler* enclosing{nullptr};
  bool has_superclass{false};
  bool is_static_method{false};
};

class Compiler {
  // Parser state (shared across all Compiler instances via pointer)
  ParseState* ps_;

  // Compiler state (per function scope)
  Compiler* enclosing_{nullptr};
  ObjFunction* function_{nullptr};
  FunctionType type_;

  Local locals_[kUINT8_COUNT];
  int local_count_{0};
  Upvalue upvalues_[kUINT8_COUNT];
  int scope_depth_{0};
  LoopContext* current_loop_{nullptr};

  // Constant folding: track last two emitted constant positions
  struct ConstRecord {
    sz_t code_offset{0};   // where OP_CONSTANT/OP_CONSTANT_LONG starts
    sz_t const_index{0};   // index into constants array
    bool valid{false};
  };
  ConstRecord last_const_;
  ConstRecord prev_const_;

  void record_constant(sz_t code_offset, sz_t const_index) noexcept;
  void invalidate_constants() noexcept;
  bool try_fold_unary(OpCode op) noexcept;
  bool try_fold_binary(OpCode op) noexcept;
  bool try_fuse_binary_local_local(OpCode op) noexcept;

  // Error reporting
  void error_at(const Token& token, strv_t message) noexcept;
  void error(strv_t message) noexcept;
  void error_at_current(strv_t message) noexcept;

  // Token consumption
  void advance() noexcept;
  void consume(TokenType type, strv_t message) noexcept;
  bool check(TokenType type) const noexcept;
  bool match(TokenType type) noexcept;

  // Bytecode emission
  Chunk& current_chunk() noexcept;
  void emit_byte(u8_t byte) noexcept;
  void emit_bytes(u8_t byte1, u8_t byte2) noexcept;
  void emit_op(OpCode op) noexcept;
  void emit_op_byte(OpCode op, u8_t byte) noexcept;
  u8_t emit_ic_slot() noexcept;
  void emit_return() noexcept;
  void emit_constant(Value value) noexcept;
  sz_t emit_jump(OpCode op) noexcept;
  void patch_jump(sz_t offset) noexcept;
  void emit_loop(sz_t loop_start) noexcept;
  sz_t make_constant(Value value) noexcept;

  // Variables
  u8_t identifier_constant(const Token& name) noexcept;
  u8_t parse_variable(strv_t error_msg) noexcept;
  void define_variable(u8_t global) noexcept;
  void declare_variable() noexcept;
  void mark_initialized() noexcept;
  void add_local(Token name) noexcept;
  int resolve_local(const Token& name) noexcept;
  int resolve_upvalue(const Token& name) noexcept;
  int add_upvalue(u8_t index, bool is_local) noexcept;
  void named_variable(const Token& name, bool can_assign) noexcept;

  // Scoping
  void begin_scope() noexcept;
  void end_scope() noexcept;

  // Parsing by precedence
  void parse_precedence(Precedence precedence) noexcept;
  const ParseRule& get_rule(TokenType type) noexcept;

  // Statements and declarations
  void expression() noexcept;
  void block() noexcept;
  void declaration() noexcept;
  void var_declaration() noexcept;
  void fun_declaration() noexcept;
  void class_declaration() noexcept;
  void import_declaration() noexcept;
  void statement() noexcept;
  void expression_statement() noexcept;
  void print_statement() noexcept;
  void if_statement() noexcept;
  void while_statement() noexcept;
  void for_statement() noexcept;
  void for_in_statement(Token var_name) noexcept;
  void list_comprehension_() noexcept;
  void break_statement() noexcept;
  void continue_statement() noexcept;
  void return_statement() noexcept;
  void try_statement() noexcept;
  void throw_statement() noexcept;
  void defer_statement() noexcept;
  void switch_statement() noexcept;
  void function(FunctionType type) noexcept;
  void method() noexcept;
  void synchronize() noexcept;

  ObjFunction* end_compiler() noexcept;

public:
  Compiler(ParseState& ps, FunctionType type) noexcept;

  // Expression parsers — public so ParseRule table can take member-function pointers
  void grouping(bool can_assign) noexcept;
  void number(bool can_assign) noexcept;
  void integer(bool can_assign) noexcept;
  void string(bool can_assign) noexcept;
  void string_interpolation(bool can_assign) noexcept;
  void variable(bool can_assign) noexcept;
  void unary(bool can_assign) noexcept;
  void binary(bool can_assign) noexcept;
  void bitwise(bool can_assign) noexcept;
  void literal(bool can_assign) noexcept;
  void and_(bool can_assign) noexcept;
  void or_(bool can_assign) noexcept;
  void call(bool can_assign) noexcept;
  void dot(bool can_assign) noexcept;
  void this_(bool can_assign) noexcept;
  void super_(bool can_assign) noexcept;
  void fun_expression(bool can_assign) noexcept;
  void list_(bool can_assign) noexcept;
  void map_(bool can_assign) noexcept;
  void subscript_(bool can_assign) noexcept;
  void ternary(bool can_assign) noexcept;
  u8_t argument_list() noexcept;

  friend ObjFunction* compile(strv_t source, strv_t script_path) noexcept;
  friend void mark_compiler_roots() noexcept;
};

static ParseState* active_parse_state_ = nullptr;

// Parse rule table — indexed by TokenType enum value.
// Order MUST match TokenTypes.hh; static_assert below verifies size.
static constexpr sz_t kTOKEN_COUNT = static_cast<sz_t>(TokenType::TOKEN_COUNT);

static std::array<ParseRule, kTOKEN_COUNT> rules = {{
  // TOKEN_LEFT_PAREN
  { &Compiler::grouping, &Compiler::call,    Precedence::PREC_CALL },
  // TOKEN_RIGHT_PAREN
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_LEFT_BRACE
  { &Compiler::map_,     nullptr,            Precedence::PREC_NONE },
  // TOKEN_RIGHT_BRACE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_LEFT_BRACKET
  { &Compiler::list_,    &Compiler::subscript_, Precedence::PREC_CALL },
  // TOKEN_RIGHT_BRACKET
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_COMMA
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_DOT
  { nullptr,             &Compiler::dot,     Precedence::PREC_CALL },
  // TOKEN_MINUS
  { &Compiler::unary,    &Compiler::binary,  Precedence::PREC_TERM },
  // TOKEN_PLUS
  { nullptr,             &Compiler::binary,  Precedence::PREC_TERM },
  // TOKEN_SEMICOLON
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_SLASH
  { nullptr,             &Compiler::binary,  Precedence::PREC_FACTOR },
  // TOKEN_STAR
  { nullptr,             &Compiler::binary,  Precedence::PREC_FACTOR },
  // TOKEN_PERCENT
  { nullptr,             &Compiler::binary,  Precedence::PREC_FACTOR },
  // TOKEN_AMPERSAND
  { nullptr,             &Compiler::bitwise, Precedence::PREC_BIT_AND },
  // TOKEN_PIPE
  { nullptr,             &Compiler::bitwise, Precedence::PREC_BIT_OR },
  // TOKEN_CARET
  { nullptr,             &Compiler::bitwise, Precedence::PREC_BIT_XOR },
  // TOKEN_TILDE
  { &Compiler::unary,    nullptr,            Precedence::PREC_NONE },
  // TOKEN_BANG
  { &Compiler::unary,    nullptr,            Precedence::PREC_NONE },
  // TOKEN_BANG_EQUAL
  { nullptr,             &Compiler::binary,  Precedence::PREC_EQUALITY },
  // TOKEN_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_EQUAL_EQUAL
  { nullptr,             &Compiler::binary,  Precedence::PREC_EQUALITY },
  // TOKEN_GREATER
  { nullptr,             &Compiler::binary,  Precedence::PREC_COMPARISON },
  // TOKEN_GREATER_EQUAL
  { nullptr,             &Compiler::binary,  Precedence::PREC_COMPARISON },
  // TOKEN_LESS
  { nullptr,             &Compiler::binary,  Precedence::PREC_COMPARISON },
  // TOKEN_LESS_EQUAL
  { nullptr,             &Compiler::binary,  Precedence::PREC_COMPARISON },
  // TOKEN_QUESTION
  { nullptr,             &Compiler::ternary, Precedence::PREC_TERNARY },
  // TOKEN_COLON
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_PLUS_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_MINUS_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_STAR_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_SLASH_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_PERCENT_EQUAL
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_LEFT_SHIFT
  { nullptr,             &Compiler::bitwise, Precedence::PREC_SHIFT },
  // TOKEN_RIGHT_SHIFT
  { nullptr,             &Compiler::bitwise, Precedence::PREC_SHIFT },
  // TOKEN_IDENTIFIER
  { &Compiler::variable, nullptr,            Precedence::PREC_NONE },
  // TOKEN_STRING
  { &Compiler::string,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_STRING_INTERP
  { &Compiler::string_interpolation, nullptr, Precedence::PREC_NONE },
  // TOKEN_NUMBER
  { &Compiler::number,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_INTEGER
  { &Compiler::integer,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_AND
  { nullptr,             &Compiler::and_,    Precedence::PREC_AND },
  // TOKEN_BREAK
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_CLASS
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_CONTINUE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_ELSE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_FALSE
  { &Compiler::literal,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_FOR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_FUN
  { &Compiler::fun_expression, nullptr,      Precedence::PREC_NONE },
  // TOKEN_IF
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_IN
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_IMPORT
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_FROM
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_AS
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_NIL
  { &Compiler::literal,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_OR
  { nullptr,             &Compiler::or_,     Precedence::PREC_OR },
  // TOKEN_PRINT
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_RETURN
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_STATIC
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_SUPER
  { &Compiler::super_,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_THIS
  { &Compiler::this_,    nullptr,            Precedence::PREC_NONE },
  // TOKEN_THROW
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_TRUE
  { &Compiler::literal,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_TRY
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_VAR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_SWITCH
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_WHILE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_CASE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_CATCH
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_DEFAULT
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_DEFER
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_ERROR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_EOF
  { nullptr,             nullptr,            Precedence::PREC_NONE },
}};

static_assert(rules.size() == kTOKEN_COUNT,
    "ParseRule table size must match TokenType enum count");

// --- Compiler implementation ---

Compiler::Compiler(ParseState& ps, FunctionType type) noexcept
    : ps_(&ps), type_(type) {
  enclosing_ = ps_->current_compiler;
  ps_->current_compiler = this;

  function_ = VM::get_instance().allocate<ObjFunction>();
  function_->set_script_path(ps_->script_path);

  if (type != FunctionType::TYPE_SCRIPT) {
    function_->set_name(VM::get_instance().copy_string(
        ps_->previous.lexeme.data(), ps_->previous.lexeme.length()));
  }

  // Slot 0 is reserved for the VM's internal use
  Local& local = locals_[local_count_++];
  local.depth = 0;
  local.is_captured = false;
  if (type != FunctionType::TYPE_FUNCTION) {
    local.name.lexeme = "this";
  } else {
    local.name.lexeme = "";
  }
}

void Compiler::record_constant(sz_t code_offset, sz_t const_index) noexcept {
  prev_const_ = last_const_;
  last_const_ = {code_offset, const_index, true};
}

void Compiler::invalidate_constants() noexcept {
  last_const_.valid = false;
  prev_const_.valid = false;
}

bool Compiler::try_fold_unary(OpCode op) noexcept {
  if (!last_const_.valid) return false;

  const Value& operand = current_chunk().constant_at(last_const_.const_index);

  if (op == OpCode::OP_NEGATE) {
    if (operand.is_integer()) {
      i64_t result = -operand.as_integer();
      current_chunk().truncate(last_const_.code_offset);
      last_const_.valid = false;
      emit_constant(Value(result));
      return true;
    }
    if (operand.is_double()) {
      double result = -operand.as_number();
      current_chunk().truncate(last_const_.code_offset);
      last_const_.valid = false;
      emit_constant(Value(result));
      return true;
    }
  }
  return false;
}

bool Compiler::try_fold_binary(OpCode op) noexcept {
  if (!last_const_.valid || !prev_const_.valid) return false;

  const Value& right = current_chunk().constant_at(last_const_.const_index);
  const Value& left = current_chunk().constant_at(prev_const_.const_index);

  // Integer folding (both operands are integers)
  if (left.is_integer() && right.is_integer()) {
    i64_t a = left.as_integer();
    i64_t b = right.as_integer();

    switch (op) {
    case OpCode::OP_ADD: {
      current_chunk().truncate(prev_const_.code_offset);
      invalidate_constants();
      emit_constant(Value(static_cast<i64_t>(a + b)));
      return true;
    }
    case OpCode::OP_SUBTRACT: {
      current_chunk().truncate(prev_const_.code_offset);
      invalidate_constants();
      emit_constant(Value(static_cast<i64_t>(a - b)));
      return true;
    }
    case OpCode::OP_MULTIPLY: {
      current_chunk().truncate(prev_const_.code_offset);
      invalidate_constants();
      emit_constant(Value(static_cast<i64_t>(a * b)));
      return true;
    }
    case OpCode::OP_DIVIDE: {
      if (b == 0) return false;
      // int / int → float
      current_chunk().truncate(prev_const_.code_offset);
      invalidate_constants();
      emit_constant(Value(static_cast<double>(a) / static_cast<double>(b)));
      return true;
    }
    case OpCode::OP_MODULO: {
      if (b == 0) return false;
      current_chunk().truncate(prev_const_.code_offset);
      invalidate_constants();
      emit_constant(Value(static_cast<i64_t>(a % b)));
      return true;
    }
    default: return false;
    }
  }

  // Number folding (at least one double)
  if (left.is_number() && right.is_number()) {
    double a = left.as_number();
    double b = right.as_number();
    double result;

    switch (op) {
    case OpCode::OP_ADD:      result = a + b; break;
    case OpCode::OP_SUBTRACT: result = a - b; break;
    case OpCode::OP_MULTIPLY: result = a * b; break;
    case OpCode::OP_DIVIDE:
      if (b == 0.0) return false;  // let VM handle division by zero
      result = a / b;
      break;
    case OpCode::OP_MODULO:
      if (b == 0.0) return false;
      result = std::fmod(a, b);
      break;
    default: return false;
    }

    // Roll back both constants + the binary op
    current_chunk().truncate(prev_const_.code_offset);
    invalidate_constants();
    emit_constant(Value(result));
    return true;
  }

  // String concatenation folding
  if (op == OpCode::OP_ADD && left.is_string() && right.is_string()) {
    str_t result = as_string(left)->value() + as_string(right)->value();
    current_chunk().truncate(prev_const_.code_offset);
    invalidate_constants();
    emit_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(result.data(), result.length()))));
    return true;
  }

  return false;
}

bool Compiler::try_fuse_binary_local_local(OpCode op) noexcept {
  // Peephole: GET_LOCAL slot1, GET_LOCAL slot2, <binary_op>
  //        → <BINARY>_LOCAL_LOCAL slot1, slot2
  Chunk& chunk = current_chunk();
  sz_t count = chunk.count();
  if (count < 4) return false;

  u8_t b3 = chunk.code_at(count - 4); // OP_GET_LOCAL (left)
  u8_t b1 = chunk.code_at(count - 2); // OP_GET_LOCAL (right)
  if (b3 != static_cast<u8_t>(OpCode::OP_GET_LOCAL) ||
      b1 != static_cast<u8_t>(OpCode::OP_GET_LOCAL))
    return false;

  u8_t slot1 = chunk.code_at(count - 3);
  u8_t slot2 = chunk.code_at(count - 1);

  OpCode fused;
  switch (op) {
  case OpCode::OP_ADD:      fused = OpCode::OP_ADD_LOCAL_LOCAL; break;
  case OpCode::OP_SUBTRACT: fused = OpCode::OP_SUBTRACT_LOCAL_LOCAL; break;
  case OpCode::OP_MULTIPLY: fused = OpCode::OP_MULTIPLY_LOCAL_LOCAL; break;
  case OpCode::OP_DIVIDE:   fused = OpCode::OP_DIVIDE_LOCAL_LOCAL; break;
  case OpCode::OP_MODULO:   fused = OpCode::OP_MODULO_LOCAL_LOCAL; break;
  default: return false;
  }

  // Rewrite: truncate the two GET_LOCALs, emit fused instruction
  chunk.truncate(count - 4);
  emit_op(fused);
  emit_byte(slot1);
  emit_byte(slot2);
  return true;
}

void Compiler::error_at(const Token& token, strv_t message) noexcept {
  if (ps_->panic_mode) return;
  ps_->panic_mode = true;

  std::cerr << std::format("[line {}] Error", token.line);

  if (token.type == TokenType::TOKEN_EOF) {
    std::cerr << " at end";
  } else if (token.type != TokenType::TOKEN_ERROR) {
    std::cerr << std::format(" at '{}'", token.lexeme);
  }

  std::cerr << ": " << message << std::endl;
  ps_->had_error = true;
}

void Compiler::error(strv_t message) noexcept {
  error_at(ps_->previous, message);
}

void Compiler::error_at_current(strv_t message) noexcept {
  error_at(ps_->current, message);
}

void Compiler::advance() noexcept {
  ps_->previous = ps_->current;

  for (;;) {
    ps_->current = ps_->scanner.scan_token();
    if (ps_->current.type != TokenType::TOKEN_ERROR) break;

    error_at_current(ps_->current.lexeme);
  }
}

void Compiler::consume(TokenType type, strv_t message) noexcept {
  if (ps_->current.type == type) {
    advance();
    return;
  }
  error_at_current(message);
}

bool Compiler::check(TokenType type) const noexcept {
  return ps_->current.type == type;
}

bool Compiler::match(TokenType type) noexcept {
  if (!check(type)) return false;
  advance();
  return true;
}

Chunk& Compiler::current_chunk() noexcept {
  return function_->chunk();
}

void Compiler::emit_byte(u8_t byte) noexcept {
  current_chunk().write(byte, ps_->previous.line,
      ps_->previous.column, static_cast<int>(ps_->previous.lexeme.size()));
}

void Compiler::emit_bytes(u8_t byte1, u8_t byte2) noexcept {
  emit_byte(byte1);
  emit_byte(byte2);
}

void Compiler::emit_op(OpCode op) noexcept {
  emit_byte(static_cast<u8_t>(op));
}

void Compiler::emit_op_byte(OpCode op, u8_t byte) noexcept {
  emit_op(op);
  emit_byte(byte);
}

u8_t Compiler::emit_ic_slot() noexcept {
  auto slot = static_cast<u8_t>(function_->add_ic());
  emit_byte(slot);
  return slot;
}

void Compiler::emit_return() noexcept {
  if (type_ == FunctionType::TYPE_INITIALIZER) {
    emit_op_byte(OpCode::OP_GET_LOCAL, 0);
  } else {
    emit_op(OpCode::OP_NIL);
  }
  emit_op(OpCode::OP_RETURN);
}

sz_t Compiler::make_constant(Value value) noexcept {
  sz_t constant = current_chunk().add_constant(value);
  if (constant > 0xFFFFFF) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return constant;
}

void Compiler::emit_constant(Value value) noexcept {
  sz_t code_offset = current_chunk().count();
  sz_t index = make_constant(value);
  if (index <= 255) {
    emit_op_byte(OpCode::OP_CONSTANT, static_cast<u8_t>(index));
  } else {
    emit_op(OpCode::OP_CONSTANT_LONG);
    emit_byte(static_cast<u8_t>(index & 0xFF));
    emit_byte(static_cast<u8_t>((index >> 8) & 0xFF));
    emit_byte(static_cast<u8_t>((index >> 16) & 0xFF));
  }
  record_constant(code_offset, index);
}

sz_t Compiler::emit_jump(OpCode op) noexcept {
  emit_op(op);
  emit_byte(0xff);
  emit_byte(0xff);
  return current_chunk().count() - 2;
}

void Compiler::patch_jump(sz_t offset) noexcept {
  sz_t jump = current_chunk().count() - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  current_chunk()[offset] = static_cast<u8_t>((jump >> 8) & 0xff);
  current_chunk()[offset + 1] = static_cast<u8_t>(jump & 0xff);
}

void Compiler::emit_loop(sz_t loop_start) noexcept {
  emit_op(OpCode::OP_LOOP);

  sz_t offset = current_chunk().count() - loop_start + 2;
  if (offset > UINT16_MAX) error("Loop body too large.");

  emit_byte(static_cast<u8_t>((offset >> 8) & 0xff));
  emit_byte(static_cast<u8_t>(offset & 0xff));
}

u8_t Compiler::identifier_constant(const Token& name) noexcept {
  sz_t index = make_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(name.lexeme.data(), name.lexeme.length()))));
  if (index > 255) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<u8_t>(index);
}

u8_t Compiler::parse_variable(strv_t error_msg) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, error_msg);

  declare_variable();
  if (scope_depth_ > 0) return 0;

  return identifier_constant(ps_->previous);
}

void Compiler::define_variable(u8_t global) noexcept {
  if (scope_depth_ > 0) {
    mark_initialized();
    return;
  }
  emit_op_byte(OpCode::OP_DEFINE_GLOBAL, global);
}

void Compiler::declare_variable() noexcept {
  if (scope_depth_ == 0) return;

  Token& name = ps_->previous;

  for (int i = local_count_ - 1; i >= 0; i--) {
    Local& local = locals_[i];
    if (local.depth != -1 && local.depth < scope_depth_) break;

    if (name.lexeme == local.name.lexeme) {
      error("Already a variable with this name in this scope.");
    }
  }

  add_local(name);
}

void Compiler::mark_initialized() noexcept {
  if (scope_depth_ == 0) return;
  locals_[local_count_ - 1].depth = scope_depth_;
}

void Compiler::add_local(Token name) noexcept {
  if (local_count_ == static_cast<int>(kUINT8_COUNT)) {
    error("Too many local variables in function.");
    return;
  }

  Local& local = locals_[local_count_++];
  local.name = name;
  local.depth = -1;
  local.is_captured = false;
}

int Compiler::resolve_local(const Token& name) noexcept {
  for (int i = local_count_ - 1; i >= 0; i--) {
    Local& local = locals_[i];
    if (name.lexeme == local.name.lexeme) {
      if (local.depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

int Compiler::add_upvalue(u8_t index, bool is_local) noexcept {
  int upvalue_count = function_->upvalue_count();

  for (int i = 0; i < upvalue_count; i++) {
    Upvalue& upvalue = upvalues_[i];
    if (upvalue.index == index && upvalue.is_local == is_local) {
      return i;
    }
  }

  if (upvalue_count == static_cast<int>(kUINT8_COUNT)) {
    error("Too many closure variables in function.");
    return 0;
  }

  upvalues_[upvalue_count].is_local = is_local;
  upvalues_[upvalue_count].index = index;
  return function_->increment_upvalue_count();
}

int Compiler::resolve_upvalue(const Token& name) noexcept {
  if (enclosing_ == nullptr) return -1;

  int local = enclosing_->resolve_local(name);
  if (local != -1) {
    enclosing_->locals_[local].is_captured = true;
    return add_upvalue(static_cast<u8_t>(local), true);
  }

  int upvalue = enclosing_->resolve_upvalue(name);
  if (upvalue != -1) {
    return add_upvalue(static_cast<u8_t>(upvalue), false);
  }

  return -1;
}

// Returns the arithmetic opcode for a compound assignment token type.
// Returns OP_RETURN as a sentinel when the token is not a compound assignment.
static OpCode compound_op(TokenType type) noexcept {
  switch (type) {
  case TokenType::TOKEN_PLUS_EQUAL:    return OpCode::OP_ADD;
  case TokenType::TOKEN_MINUS_EQUAL:   return OpCode::OP_SUBTRACT;
  case TokenType::TOKEN_STAR_EQUAL:    return OpCode::OP_MULTIPLY;
  case TokenType::TOKEN_SLASH_EQUAL:   return OpCode::OP_DIVIDE;
  case TokenType::TOKEN_PERCENT_EQUAL: return OpCode::OP_MODULO;
  default: return OpCode::OP_RETURN; // sentinel: not a compound op
  }
}

static bool is_compound_op(TokenType type) noexcept {
  return compound_op(type) != OpCode::OP_RETURN;
}

void Compiler::named_variable(const Token& name, bool can_assign) noexcept {
  OpCode get_op, set_op;
  int arg = resolve_local(name);
  if (arg != -1) {
    get_op = OpCode::OP_GET_LOCAL;
    set_op = OpCode::OP_SET_LOCAL;
  } else if ((arg = resolve_upvalue(name)) != -1) {
    get_op = OpCode::OP_GET_UPVALUE;
    set_op = OpCode::OP_SET_UPVALUE;
  } else {
    arg = identifier_constant(name);
    get_op = OpCode::OP_GET_GLOBAL;
    set_op = OpCode::OP_SET_GLOBAL;
  }

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    emit_op_byte(set_op, static_cast<u8_t>(arg));
  } else if (can_assign && is_compound_op(ps_->current.type)) {
    OpCode op = compound_op(ps_->current.type);
    advance();
    emit_op_byte(get_op, static_cast<u8_t>(arg));
    expression();
    emit_op(op);
    emit_op_byte(set_op, static_cast<u8_t>(arg));
  } else {
    emit_op_byte(get_op, static_cast<u8_t>(arg));
  }
}

void Compiler::begin_scope() noexcept {
  scope_depth_++;
}

void Compiler::end_scope() noexcept {
  scope_depth_--;

  while (local_count_ > 0 &&
         locals_[local_count_ - 1].depth > scope_depth_) {
    if (locals_[local_count_ - 1].is_captured) {
      emit_op(OpCode::OP_CLOSE_UPVALUE);
    } else {
      emit_op(OpCode::OP_POP);
    }
    local_count_--;
  }
}

const ParseRule& Compiler::get_rule(TokenType type) noexcept {
  return rules[static_cast<int>(type)];
}

void Compiler::parse_precedence(Precedence precedence) noexcept {
  advance();
  ParseFn prefix_rule = get_rule(ps_->previous.type).prefix;
  if (prefix_rule == nullptr) {
    error("Expect expression.");
    return;
  }

  bool can_assign = (precedence <= Precedence::PREC_ASSIGNMENT);
  (this->*prefix_rule)(can_assign);

  while (precedence <= get_rule(ps_->current.type).precedence) {
    advance();
    ParseFn infix_rule = get_rule(ps_->previous.type).infix;
    (this->*infix_rule)(can_assign);
  }

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

void Compiler::expression() noexcept {
  parse_precedence(Precedence::PREC_ASSIGNMENT);
}

void Compiler::grouping(bool /*can_assign*/) noexcept {
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::number(bool /*can_assign*/) noexcept {
  double value = 0.0;
  auto [ptr, ec] = std::from_chars(
      ps_->previous.lexeme.data(),
      ps_->previous.lexeme.data() + ps_->previous.lexeme.size(),
      value);
  if (ec != std::errc{}) {
    error("Invalid number literal.");
  }
  emit_constant(Value(value));
}

void Compiler::integer(bool /*can_assign*/) noexcept {
  i64_t value = 0;
  auto [ptr, ec] = std::from_chars(
      ps_->previous.lexeme.data(),
      ps_->previous.lexeme.data() + ps_->previous.lexeme.size(),
      value);
  if (ec != std::errc{}) {
    error("Invalid integer literal.");
  }
  emit_constant(Value(value));
}

void Compiler::string(bool /*can_assign*/) noexcept {
  // Trim the surrounding quotes
  const char* start = ps_->previous.lexeme.data() + 1;
  sz_t length = ps_->previous.lexeme.length() - 2;

  str_t result;
  result.reserve(length);
  for (sz_t i = 0; i < length; ++i) {
    if (start[i] == '\\' && i + 1 < length) {
      ++i;
      switch (start[i]) {
      case 'n':  result += '\n'; break;
      case 't':  result += '\t'; break;
      case 'r':  result += '\r'; break;
      case '\\': result += '\\'; break;
      case '"':  result += '"';  break;
      case '0':  result += '\0'; break;
      case '$':  result += '$';  break;
      default:   result += '\\'; result += start[i]; break;
      }
    } else {
      result += start[i];
    }
  }

  emit_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(result.data(), result.length()))));
}

void Compiler::string_interpolation(bool /*can_assign*/) noexcept {
  // Helper lambda: process escape sequences in a raw segment
  auto process_escapes = [](const char* src, sz_t len) -> str_t {
    str_t result;
    result.reserve(len);
    for (sz_t i = 0; i < len; ++i) {
      if (src[i] == '\\' && i + 1 < len) {
        ++i;
        switch (src[i]) {
        case 'n':  result += '\n'; break;
        case 't':  result += '\t'; break;
        case 'r':  result += '\r'; break;
        case '\\': result += '\\'; break;
        case '"':  result += '"';  break;
        case '0':  result += '\0'; break;
        case '$':  result += '$';  break;
        default:   result += '\\'; result += src[i]; break;
        }
      } else {
        result += src[i];
      }
    }
    return result;
  };

  // Helper lambda: emit a string constant for a segment
  auto emit_segment = [&](const char* src, sz_t len) {
    str_t seg = process_escapes(src, len);
    emit_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(seg.data(), seg.length()))));
  };

  // First segment: lexeme starts with '"', strip it
  const char* first_start = ps_->previous.lexeme.data() + 1;
  sz_t first_len = ps_->previous.lexeme.length() - 1;

  bool has_prev = (first_len > 0);
  if (has_prev) {
    emit_segment(first_start, first_len);
  }

  for (;;) {
    // Parse the interpolated expression
    expression();
    // Convert expression result to string
    emit_op(OpCode::OP_STR);
    if (has_prev) {
      emit_op(OpCode::OP_ADD);
    }
    has_prev = true;

    if (match(TokenType::TOKEN_STRING_INTERP)) {
      // Intermediate segment: no quotes in lexeme
      const char* seg_start = ps_->previous.lexeme.data();
      sz_t seg_len = ps_->previous.lexeme.length();
      if (seg_len > 0) {
        emit_segment(seg_start, seg_len);
        emit_op(OpCode::OP_ADD);
      }
    } else if (match(TokenType::TOKEN_STRING)) {
      // Final segment: no quotes in lexeme (scan_string_continuation
      // produces lexeme from after '}' to before '"')
      const char* seg_start = ps_->previous.lexeme.data();
      sz_t seg_len = ps_->previous.lexeme.length();
      if (seg_len > 0) {
        emit_segment(seg_start, seg_len);
        emit_op(OpCode::OP_ADD);
      }
      break;
    } else {
      error("Unterminated string interpolation.");
      break;
    }
  }
  invalidate_constants();
}

void Compiler::variable(bool can_assign) noexcept {
  named_variable(ps_->previous, can_assign);
  invalidate_constants();
}

void Compiler::unary(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;

  parse_precedence(Precedence::PREC_UNARY);

  switch (operator_type) {
  case TokenType::TOKEN_BANG:
    emit_op(OpCode::OP_NOT);
    invalidate_constants();
    break;
  case TokenType::TOKEN_MINUS:
    emit_op(OpCode::OP_NEGATE);
    if (!try_fold_unary(OpCode::OP_NEGATE)) {
      invalidate_constants();
    }
    break;
  case TokenType::TOKEN_TILDE:
    emit_op(OpCode::OP_BITNOT);
    invalidate_constants();
    break;
  default: return;
  }
}

void Compiler::binary(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;
  const ParseRule& rule = get_rule(operator_type);

  // Save the left operand's constant record before parsing the right side,
  // since parse_precedence may overwrite prev_const_ during sub-expression folding.
  ConstRecord left_const = last_const_;

  parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

  // Restore: right operand is now in last_const_, set left as prev_const_
  prev_const_ = left_const;

  // Map operator token to binary opcode (for foldable/fusible arithmetic ops)
  OpCode arith_op = OpCode::OP_RETURN; // sentinel: not arithmetic
  switch (operator_type) {
  case TokenType::TOKEN_PLUS:    arith_op = OpCode::OP_ADD; break;
  case TokenType::TOKEN_MINUS:   arith_op = OpCode::OP_SUBTRACT; break;
  case TokenType::TOKEN_STAR:    arith_op = OpCode::OP_MULTIPLY; break;
  case TokenType::TOKEN_SLASH:   arith_op = OpCode::OP_DIVIDE; break;
  case TokenType::TOKEN_PERCENT: arith_op = OpCode::OP_MODULO; break;
  default: break;
  }

  // Try superinstruction fusion before emitting the binary op
  if (arith_op != OpCode::OP_RETURN && try_fuse_binary_local_local(arith_op)) {
    invalidate_constants();
    return;
  }

  switch (operator_type) {
  case TokenType::TOKEN_BANG_EQUAL:    emit_bytes(static_cast<u8_t>(OpCode::OP_EQUAL), static_cast<u8_t>(OpCode::OP_NOT)); break;
  case TokenType::TOKEN_EQUAL_EQUAL:   emit_op(OpCode::OP_EQUAL); break;
  case TokenType::TOKEN_GREATER:       emit_op(OpCode::OP_GREATER); break;
  case TokenType::TOKEN_GREATER_EQUAL: emit_bytes(static_cast<u8_t>(OpCode::OP_LESS), static_cast<u8_t>(OpCode::OP_NOT)); break;
  case TokenType::TOKEN_LESS:          emit_op(OpCode::OP_LESS); break;
  case TokenType::TOKEN_LESS_EQUAL:    emit_bytes(static_cast<u8_t>(OpCode::OP_GREATER), static_cast<u8_t>(OpCode::OP_NOT)); break;
  case TokenType::TOKEN_PLUS:          emit_op(OpCode::OP_ADD); break;
  case TokenType::TOKEN_MINUS:         emit_op(OpCode::OP_SUBTRACT); break;
  case TokenType::TOKEN_STAR:          emit_op(OpCode::OP_MULTIPLY); break;
  case TokenType::TOKEN_SLASH:         emit_op(OpCode::OP_DIVIDE); break;
  case TokenType::TOKEN_PERCENT:       emit_op(OpCode::OP_MODULO); break;
  default: return;
  }

  if (arith_op != OpCode::OP_RETURN) {
    if (!try_fold_binary(arith_op)) {
      invalidate_constants();
    }
  } else {
    invalidate_constants();
  }
}

void Compiler::bitwise(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;
  const ParseRule& rule = get_rule(operator_type);
  parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

  switch (operator_type) {
  case TokenType::TOKEN_AMPERSAND:   emit_op(OpCode::OP_BITAND); break;
  case TokenType::TOKEN_PIPE:        emit_op(OpCode::OP_BITOR); break;
  case TokenType::TOKEN_CARET:       emit_op(OpCode::OP_BITXOR); break;
  case TokenType::TOKEN_LEFT_SHIFT:  emit_op(OpCode::OP_LSHIFT); break;
  case TokenType::TOKEN_RIGHT_SHIFT: emit_op(OpCode::OP_RSHIFT); break;
  default: return;
  }
  invalidate_constants();
}

void Compiler::literal(bool /*can_assign*/) noexcept {
  switch (ps_->previous.type) {
  case TokenType::TOKEN_FALSE: emit_op(OpCode::OP_FALSE); break;
  case TokenType::TOKEN_NIL:   emit_op(OpCode::OP_NIL); break;
  case TokenType::TOKEN_TRUE:  emit_op(OpCode::OP_TRUE); break;
  default: return;
  }
  invalidate_constants();
}

void Compiler::and_(bool /*can_assign*/) noexcept {
  sz_t end_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);

  emit_op(OpCode::OP_POP);
  parse_precedence(Precedence::PREC_AND);

  patch_jump(end_jump);
  invalidate_constants();
}

void Compiler::or_(bool /*can_assign*/) noexcept {
  sz_t else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  sz_t end_jump = emit_jump(OpCode::OP_JUMP);

  patch_jump(else_jump);
  emit_op(OpCode::OP_POP);

  parse_precedence(Precedence::PREC_OR);
  patch_jump(end_jump);
  invalidate_constants();
}

void Compiler::ternary(bool /*can_assign*/) noexcept {
  // condition already on the stack
  sz_t then_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  emit_op(OpCode::OP_POP);

  // then branch: parse expression for the truthy value
  parse_precedence(Precedence::PREC_TERNARY);
  sz_t else_jump = emit_jump(OpCode::OP_JUMP);

  patch_jump(then_jump);
  emit_op(OpCode::OP_POP);

  consume(TokenType::TOKEN_COLON, "Expect ':' after ternary then branch.");

  // else branch: parse expression for the falsy value
  parse_precedence(Precedence::PREC_TERNARY);
  patch_jump(else_jump);
  invalidate_constants();
}

u8_t Compiler::argument_list() noexcept {
  u8_t arg_count = 0;
  if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (arg_count == 255) {
        error("Can't have more than 255 arguments.");
      }
      arg_count++;
    } while (match(TokenType::TOKEN_COMMA));
  }
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  return arg_count;
}

void Compiler::call(bool /*can_assign*/) noexcept {
  u8_t arg_count = argument_list();
  emit_op_byte(OpCode::OP_CALL, arg_count);
  invalidate_constants();
}

void Compiler::dot(bool can_assign) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect property name after '.'.");
  u8_t name = identifier_constant(ps_->previous);

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    emit_op_byte(OpCode::OP_SET_PROPERTY, name);
    emit_ic_slot();
  } else if (match(TokenType::TOKEN_LEFT_PAREN)) {
    u8_t arg_count = argument_list();
    emit_bytes(static_cast<u8_t>(OpCode::OP_INVOKE), name);
    emit_byte(arg_count);
    emit_ic_slot();
  } else {
    emit_op_byte(OpCode::OP_GET_PROPERTY, name);
    emit_ic_slot();
  }
  invalidate_constants();
}

void Compiler::this_(bool /*can_assign*/) noexcept {
  if (ps_->current_class == nullptr) {
    error("Can't use 'this' outside of a class.");
    return;
  }
  if (ps_->current_class->is_static_method) {
    error("Can't use 'this' in a static method.");
    return;
  }
  variable(false);
}

void Compiler::super_(bool /*can_assign*/) noexcept {
  if (ps_->current_class == nullptr) {
    error("Can't use 'super' outside of a class.");
  } else if (ps_->current_class->is_static_method) {
    error("Can't use 'super' in a static method.");
  } else if (!ps_->current_class->has_superclass) {
    error("Can't use 'super' in a class with no superclass.");
  }

  consume(TokenType::TOKEN_DOT, "Expect '.' after 'super'.");
  consume(TokenType::TOKEN_IDENTIFIER, "Expect superclass method name.");
  u8_t name = identifier_constant(ps_->previous);

  named_variable(Token{TokenType::TOKEN_THIS, "this", ps_->previous.line}, false);

  if (match(TokenType::TOKEN_LEFT_PAREN)) {
    u8_t arg_count = argument_list();
    named_variable(Token{TokenType::TOKEN_SUPER, "super", ps_->previous.line}, false);
    emit_bytes(static_cast<u8_t>(OpCode::OP_SUPER_INVOKE), name);
    emit_byte(arg_count);
  } else {
    named_variable(Token{TokenType::TOKEN_SUPER, "super", ps_->previous.line}, false);
    emit_op_byte(OpCode::OP_GET_SUPER, name);
  }
  invalidate_constants();
}

void Compiler::list_(bool /*can_assign*/) noexcept {
  if (check(TokenType::TOKEN_RIGHT_BRACKET)) {
    // Empty list: []
    consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");
    emit_op_byte(OpCode::OP_BUILD_LIST, 0);
    invalidate_constants();
    return;
  }

  // Save state before parsing first expression to detect comprehension
  auto scanner_state = ps_->scanner.save_state();
  Token saved_current = ps_->current;
  Token saved_previous = ps_->previous;
  sz_t saved_code_count = current_chunk().count();
  ConstRecord saved_last_const = last_const_;
  ConstRecord saved_prev_const = prev_const_;

  expression();

  if (match(TokenType::TOKEN_FOR)) {
    // List comprehension: [expr for var in iterable]
    // Rewind the emitted expression bytecode
    current_chunk().truncate(saved_code_count);
    ps_->scanner.restore_state(scanner_state);
    ps_->current = saved_current;
    ps_->previous = saved_previous;
    last_const_ = saved_last_const;
    prev_const_ = saved_prev_const;

    list_comprehension_();
    return;
  }

  // Normal list literal
  u8_t count = 1;
  while (match(TokenType::TOKEN_COMMA)) {
    expression();
    if (count == 255) {
      error("Can't have more than 255 elements in a list literal.");
    }
    count++;
  }
  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");
  emit_op_byte(OpCode::OP_BUILD_LIST, count);
  invalidate_constants();
}

void Compiler::list_comprehension_() noexcept {
  // Syntax: [expr for var in iterable]
  //         [expr for var in iterable if condition]
  //
  // Bytecode layout:
  //   OP_BUILD_LIST 0          → __list (slot N)
  //   <iterable expr>          → __seq  (slot N+1)
  //   OP_CONSTANT 0            → __idx  (slot N+2)
  //   loop_start:
  //   OP_FOR_ITER N+1, exit    → pushes element or jumps
  //   <element on stack>       → loop variable (slot N+3)
  //   [if condition: OP_JUMP_IF_FALSE skip, OP_POP]
  //   OP_GET_LOCAL N            → push __list ref
  //   <expression>              → compute element value
  //   OP_INVOKE "push" 1
  //   OP_POP                    → discard nil
  //   [skip: OP_POP (condition)]
  //   end inner scope           → pops loop variable
  //   OP_LOOP loop_start
  //   exit:
  //   OP_GET_LOCAL N            → push __list ref (result)
  //   end outer scope           → pops __idx, __seq, __list

  begin_scope(); // outer scope for __list, __seq, __idx

  // Create empty list as hidden local __list
  emit_op_byte(OpCode::OP_BUILD_LIST, 0);
  Token list_token{TokenType::TOKEN_IDENTIFIER, " __list", ps_->previous.line};
  add_local(list_token);
  mark_initialized();
  int list_slot = local_count_ - 1;

  // Save source position for the element expression (we'll parse it later)
  // First, skip past the expression to find "for var in iterable"
  // We need to parse: <expr> for <var> in <iterable> [if <cond>] ]
  // The expression will be re-parsed inside the loop body.

  // Skip the element expression (we already rewound, so current token is at expr start)
  auto expr_scanner_state = ps_->scanner.save_state();
  Token expr_current = ps_->current;
  Token expr_previous = ps_->previous;

  // Parse and discard the expression to advance past it
  {
    sz_t discard_start = current_chunk().count();
    ConstRecord discard_last = last_const_;
    ConstRecord discard_prev = prev_const_;
    expression();
    current_chunk().truncate(discard_start);
    last_const_ = discard_last;
    prev_const_ = discard_prev;
  }

  // Now we should be at "for"
  consume(TokenType::TOKEN_FOR, "Expect 'for' in list comprehension.");
  Token var_name = ps_->current;
  consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name after 'for'.");
  consume(TokenType::TOKEN_IN, "Expect 'in' after variable name.");

  // Compile the iterable expression
  expression();

  // Create hidden local for iterable (__seq)
  Token seq_token{TokenType::TOKEN_IDENTIFIER, " __seq", ps_->previous.line};
  add_local(seq_token);
  mark_initialized();
  int seq_slot = local_count_ - 1;

  // Create hidden local for index (__idx = 0)
  emit_constant(Value(0.0));
  Token idx_token{TokenType::TOKEN_IDENTIFIER, " __idx", ps_->previous.line};
  add_local(idx_token);
  mark_initialized();

  // Check for optional "if" condition — save state for later
  bool has_filter = false;
  ScannerState filter_scanner_state{};
  Token filter_current{};
  Token filter_previous{};

  if (match(TokenType::TOKEN_IF)) {
    has_filter = true;
    filter_scanner_state = ps_->scanner.save_state();
    filter_current = ps_->current;
    filter_previous = ps_->previous;

    // Skip the condition expression
    sz_t discard_start = current_chunk().count();
    ConstRecord discard_last = last_const_;
    ConstRecord discard_prev = prev_const_;
    expression();
    current_chunk().truncate(discard_start);
    last_const_ = discard_last;
    prev_const_ = discard_prev;
  }

  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list comprehension.");

  // Save scanner/parser state after consuming ']' so we can restore after re-parsing
  auto after_scanner_state = ps_->scanner.save_state();
  Token after_current = ps_->current;
  Token after_previous = ps_->previous;

  // Loop start
  sz_t loop_start = current_chunk().count();

  // OP_FOR_ITER: slot, jump_hi, jump_lo
  emit_op_byte(OpCode::OP_FOR_ITER, static_cast<u8_t>(seq_slot));
  emit_bytes(0xFF, 0xFF);
  sz_t exit_jump = current_chunk().count() - 2;

  // Inner scope for loop variable
  begin_scope();
  add_local(var_name);
  mark_initialized();

  if (has_filter) {
    // Compile the filter condition
    ps_->scanner.restore_state(filter_scanner_state);
    ps_->current = filter_current;
    ps_->previous = filter_previous;
    expression();

    sz_t skip_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_op(OpCode::OP_POP); // pop condition (true)

    // Push list ref and compile element expression
    emit_op_byte(OpCode::OP_GET_LOCAL, static_cast<u8_t>(list_slot));
    ps_->scanner.restore_state(expr_scanner_state);
    ps_->current = expr_current;
    ps_->previous = expr_previous;
    expression();

    // Invoke push
    u8_t push_name = identifier_constant(Token{TokenType::TOKEN_IDENTIFIER, "push", ps_->previous.line});
    emit_bytes(static_cast<u8_t>(OpCode::OP_INVOKE), push_name);
    emit_byte(1);
    emit_ic_slot();
    emit_op(OpCode::OP_POP); // discard nil

    sz_t end_jump = emit_jump(OpCode::OP_JUMP);
    patch_jump(skip_jump);
    emit_op(OpCode::OP_POP); // pop condition (false)
    patch_jump(end_jump);
  } else {
    // No filter — push list ref and compile element expression
    emit_op_byte(OpCode::OP_GET_LOCAL, static_cast<u8_t>(list_slot));
    ps_->scanner.restore_state(expr_scanner_state);
    ps_->current = expr_current;
    ps_->previous = expr_previous;
    expression();

    // Invoke push
    u8_t push_name = identifier_constant(Token{TokenType::TOKEN_IDENTIFIER, "push", ps_->previous.line});
    emit_bytes(static_cast<u8_t>(OpCode::OP_INVOKE), push_name);
    emit_byte(1);
    emit_ic_slot();
    emit_op(OpCode::OP_POP); // discard nil
  }

  end_scope(); // pops loop variable

  emit_loop(loop_start);

  // Patch the exit jump
  sz_t jump = current_chunk().count() - exit_jump - 2;
  current_chunk()[exit_jump] = static_cast<u8_t>((jump >> 8) & 0xFF);
  current_chunk()[exit_jump + 1] = static_cast<u8_t>(jump & 0xFF);

  // Push the list reference as the expression result
  emit_op_byte(OpCode::OP_GET_LOCAL, static_cast<u8_t>(list_slot));

  end_scope(); // pops __idx, __seq, __list (3 POPs)
  // The GET_LOCAL result survives: stack has [..., list]

  // Restore scanner/parser state to after ']'
  ps_->scanner.restore_state(after_scanner_state);
  ps_->current = after_current;
  ps_->previous = after_previous;

  invalidate_constants();
}

void Compiler::map_(bool /*can_assign*/) noexcept {
  u8_t count = 0;
  if (!check(TokenType::TOKEN_RIGHT_BRACE)) {
    do {
      expression();
      consume(TokenType::TOKEN_COLON, "Expect ':' after map key.");
      expression();
      if (count == 255) {
        error("Can't have more than 255 entries in a map literal.");
      }
      count++;
    } while (match(TokenType::TOKEN_COMMA));
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after map entries.");
  emit_op_byte(OpCode::OP_BUILD_MAP, count);
  invalidate_constants();
}

void Compiler::subscript_(bool can_assign) noexcept {
  expression();
  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after index.");

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    emit_op(OpCode::OP_INDEX_SET);
  } else {
    emit_op(OpCode::OP_INDEX_GET);
  }
  invalidate_constants();
}

void Compiler::block() noexcept {
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    declaration();
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::var_declaration() noexcept {
  u8_t global = parse_variable("Expect variable name.");

  if (match(TokenType::TOKEN_EQUAL)) {
    expression();
  } else {
    emit_op(OpCode::OP_NIL);
  }
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  define_variable(global);
}

void Compiler::function(FunctionType type) noexcept {
  Compiler compiler(*ps_, type);
  compiler.begin_scope();

  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after function name.");
  if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
    do {
      compiler.function_->increment_arity();
      if (compiler.function_->arity() > 255) {
        compiler.error_at_current("Can't have more than 255 parameters.");
      }
      u8_t constant = compiler.parse_variable("Expect parameter name.");
      compiler.define_variable(constant);
    } while (match(TokenType::TOKEN_COMMA));
  }
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  compiler.block();

  ObjFunction* function = compiler.end_compiler();
  sz_t closure_index = make_constant(Value(static_cast<Object*>(function)));
  if (closure_index > 255) {
    error("Too many constants in one chunk.");
    return;
  }
  emit_op_byte(OpCode::OP_CLOSURE, static_cast<u8_t>(closure_index));

  for (int i = 0; i < function->upvalue_count(); i++) {
    emit_byte(compiler.upvalues_[i].is_local ? 1 : 0);
    emit_byte(compiler.upvalues_[i].index);
  }
}

void Compiler::fun_expression([[maybe_unused]] bool can_assign) noexcept {
  // Synthesize a name token so the Compiler constructor picks it up.
  ps_->previous = Token{TokenType::TOKEN_IDENTIFIER, "<lambda>", ps_->previous.line};
  function(FunctionType::TYPE_FUNCTION);
  invalidate_constants();
}

void Compiler::fun_declaration() noexcept {
  u8_t global = parse_variable("Expect function name.");
  mark_initialized();
  function(FunctionType::TYPE_FUNCTION);
  define_variable(global);
}

void Compiler::method() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect method name.");
  u8_t constant = identifier_constant(ps_->previous);

  FunctionType type = FunctionType::TYPE_METHOD;
  if (ps_->previous.lexeme == "init") {
    type = FunctionType::TYPE_INITIALIZER;
  }
  function(type);

  emit_op_byte(OpCode::OP_METHOD, constant);
}

void Compiler::class_declaration() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect class name.");
  Token class_name = ps_->previous;
  u8_t name_constant = identifier_constant(ps_->previous);
  declare_variable();

  emit_op_byte(OpCode::OP_CLASS, name_constant);
  define_variable(name_constant);

  ClassCompiler class_compiler;
  class_compiler.enclosing = ps_->current_class;
  class_compiler.has_superclass = false;
  ps_->current_class = &class_compiler;

  if (match(TokenType::TOKEN_COLON)) {
    consume(TokenType::TOKEN_IDENTIFIER, "Expect superclass name.");
    variable(false);

    if (class_name.lexeme == ps_->previous.lexeme) {
      error("A class can't inherit from itself.");
    }

    begin_scope();
    add_local(Token{TokenType::TOKEN_SUPER, "super", ps_->previous.line});
    define_variable(0);

    named_variable(class_name, false);
    emit_op(OpCode::OP_INHERIT);
    class_compiler.has_superclass = true;
  }

  named_variable(class_name, false);
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before class body.");
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    if (match(TokenType::TOKEN_STATIC)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect static method name.");
      u8_t constant = identifier_constant(ps_->previous);

      if (ps_->previous.lexeme == "init") {
        error("Initializer can't be static.");
      }

      class_compiler.is_static_method = true;
      function(FunctionType::TYPE_FUNCTION);
      class_compiler.is_static_method = false;

      emit_op_byte(OpCode::OP_STATIC_METHOD, constant);
    } else if (check(TokenType::TOKEN_IDENTIFIER) &&
               (ps_->current.lexeme == "get" || ps_->current.lexeme == "set")) {
      bool is_getter = ps_->current.lexeme == "get";
      advance(); // consume "get" or "set"
      consume(TokenType::TOKEN_IDENTIFIER, "Expect property name.");
      u8_t constant = identifier_constant(ps_->previous);

      if (is_getter) {
        // Getter: no parameters, compiled as 0-arity method
        function(FunctionType::TYPE_METHOD);
        emit_op_byte(OpCode::OP_GETTER, constant);
      } else {
        // Setter: exactly one parameter
        function(FunctionType::TYPE_METHOD);
        emit_op_byte(OpCode::OP_SETTER, constant);
      }
    } else {
      method();
    }
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
  emit_op(OpCode::OP_POP);

  if (class_compiler.has_superclass) {
    end_scope();
  }

  ps_->current_class = ps_->current_class->enclosing;
}

void Compiler::import_declaration() noexcept {
  consume(TokenType::TOKEN_STRING, "Expect module path string.");
  // Trim quotes from the string
  emit_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(ps_->previous.lexeme.data() + 1,
                     ps_->previous.lexeme.length() - 2))));

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import path.");

  emit_op(OpCode::OP_IMPORT);
}

void Compiler::expression_statement() noexcept {
  expression();
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after expression.");
  emit_op(OpCode::OP_POP);
}

void Compiler::print_statement() noexcept {
  expression();
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after value.");
  emit_op(OpCode::OP_PRINT);
}

void Compiler::if_statement() noexcept {
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  sz_t then_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  emit_op(OpCode::OP_POP);
  statement();

  sz_t else_jump = emit_jump(OpCode::OP_JUMP);

  patch_jump(then_jump);
  emit_op(OpCode::OP_POP);

  if (match(TokenType::TOKEN_ELSE)) statement();
  patch_jump(else_jump);
}

void Compiler::while_statement() noexcept {
  LoopContext loop;
  loop.enclosing = current_loop_;
  loop.break_depth = scope_depth_;
  loop.continue_depth = scope_depth_;

  sz_t loop_start = current_chunk().count();
  loop.continue_target = loop_start;

  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  sz_t exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  emit_op(OpCode::OP_POP);

  current_loop_ = &loop;
  statement();
  current_loop_ = loop.enclosing;

  emit_loop(loop_start);

  patch_jump(exit_jump);
  emit_op(OpCode::OP_POP);

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::for_statement() noexcept {
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

  // Check for for-in: for (var x in expr)
  if (match(TokenType::TOKEN_VAR)) {
    // Consume the variable name
    consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name.");
    Token var_name = ps_->previous;

    if (match(TokenType::TOKEN_IN)) {
      // for-in loop: for (var <name> in <expr>) <body>
      for_in_statement(var_name);
      return;
    }

    // Standard for with var initializer.
    // We already consumed "var <name>", now handle "= expr ;" inline.
    LoopContext loop;
    loop.enclosing = current_loop_;
    loop.break_depth = scope_depth_;

    begin_scope();
    loop.continue_depth = scope_depth_;

    // Declare the variable we already consumed
    declare_variable();
    if (match(TokenType::TOKEN_EQUAL)) {
      expression();
    } else {
      emit_op(OpCode::OP_NIL);
    }
    mark_initialized();
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    // Continue with condition, increment, body (same as standard for)
    sz_t loop_start = current_chunk().count();

    sz_t exit_jump = static_cast<sz_t>(-1);
    if (!match(TokenType::TOKEN_SEMICOLON)) {
      expression();
      consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after loop condition.");
      exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
      emit_op(OpCode::OP_POP);
    }

    if (!match(TokenType::TOKEN_RIGHT_PAREN)) {
      sz_t body_jump = emit_jump(OpCode::OP_JUMP);
      sz_t increment_start = current_chunk().count();
      expression();
      emit_op(OpCode::OP_POP);
      consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
      emit_loop(loop_start);
      loop_start = increment_start;
      patch_jump(body_jump);
    }

    loop.continue_target = loop_start;
    current_loop_ = &loop;
    statement();
    current_loop_ = loop.enclosing;

    emit_loop(loop_start);

    if (exit_jump != static_cast<sz_t>(-1)) {
      patch_jump(exit_jump);
      emit_op(OpCode::OP_POP);
    }

    end_scope();

    for (sz_t offset : loop.break_jumps) {
      patch_jump(offset);
    }
    return;
  }

  // Standard C-style for loop (no var initializer)
  LoopContext loop;
  loop.enclosing = current_loop_;
  loop.break_depth = scope_depth_;

  begin_scope();
  loop.continue_depth = scope_depth_;

  // Initializer
  if (match(TokenType::TOKEN_SEMICOLON)) {
    // No initializer.
  } else {
    expression_statement();
  }

  sz_t loop_start = current_chunk().count();

  // Condition
  sz_t exit_jump = static_cast<sz_t>(-1);
  if (!match(TokenType::TOKEN_SEMICOLON)) {
    expression();
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_op(OpCode::OP_POP);
  }

  // Increment
  if (!match(TokenType::TOKEN_RIGHT_PAREN)) {
    sz_t body_jump = emit_jump(OpCode::OP_JUMP);
    sz_t increment_start = current_chunk().count();
    expression();
    emit_op(OpCode::OP_POP);
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emit_loop(loop_start);
    loop_start = increment_start;
    patch_jump(body_jump);
  }

  loop.continue_target = loop_start;

  current_loop_ = &loop;
  statement();
  current_loop_ = loop.enclosing;

  emit_loop(loop_start);

  if (exit_jump != static_cast<sz_t>(-1)) {
    patch_jump(exit_jump);
    emit_op(OpCode::OP_POP);
  }

  end_scope();

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::for_in_statement(Token var_name) noexcept {
  // Called after "for (var <name> in" has been consumed.
  //
  // Bytecode layout:
  //   <iterable expr>          → hidden local __seq (slot N)
  //   OP_CONSTANT 0            → hidden local __idx (slot N+1)
  //   loop_start:
  //   OP_FOR_ITER N, jump      → pushes element or jumps to exit
  //   <element on stack>       → loop variable (slot N+2)
  //   <body>
  //   end inner scope          → pops loop variable
  //   OP_LOOP loop_start
  //   exit:
  //   end outer scope          → pops __idx, __seq

  LoopContext loop;
  loop.enclosing = current_loop_;
  loop.break_depth = scope_depth_;

  begin_scope(); // outer scope for __seq and __idx

  // Compile the iterable expression
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after for-in clause.");

  // Create hidden local for iterable (__seq)
  Token seq_token{TokenType::TOKEN_IDENTIFIER, " __seq", 0};
  add_local(seq_token);
  mark_initialized();
  int seq_slot = local_count_ - 1;

  // Create hidden local for index (__idx = 0)
  emit_constant(Value(0.0));
  Token idx_token{TokenType::TOKEN_IDENTIFIER, " __idx", 0};
  add_local(idx_token);
  mark_initialized();

  // Loop start
  sz_t loop_start = current_chunk().count();
  loop.continue_target = loop_start;
  loop.continue_depth = scope_depth_;

  // OP_FOR_ITER: slot, jump_hi, jump_lo
  emit_op_byte(OpCode::OP_FOR_ITER, static_cast<u8_t>(seq_slot));
  // Emit placeholder jump offset (2 bytes)
  emit_bytes(0xFF, 0xFF);
  sz_t exit_jump = current_chunk().count() - 2;

  // Inner scope for loop variable
  begin_scope();

  // The element is already on the stack from OP_FOR_ITER
  add_local(var_name);
  mark_initialized();

  // Body
  current_loop_ = &loop;
  statement();
  current_loop_ = loop.enclosing;

  end_scope(); // pops loop variable

  emit_loop(loop_start);

  // Patch the exit jump
  sz_t jump = current_chunk().count() - exit_jump - 2;
  current_chunk()[exit_jump] = static_cast<u8_t>((jump >> 8) & 0xFF);
  current_chunk()[exit_jump + 1] = static_cast<u8_t>(jump & 0xFF);

  end_scope(); // pops __idx and __seq

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::break_statement() noexcept {
  if (current_loop_ == nullptr) {
    error("Can't use 'break' outside of a loop or switch.");
    return;
  }

  // Pop locals deeper than the loop's outer scope
  for (int i = local_count_ - 1; i >= 0 && locals_[i].depth > current_loop_->break_depth; i--) {
    if (locals_[i].is_captured) {
      emit_op(OpCode::OP_CLOSE_UPVALUE);
    } else {
      emit_op(OpCode::OP_POP);
    }
  }

  sz_t jump = emit_jump(OpCode::OP_JUMP);
  current_loop_->break_jumps.push_back(jump);

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after 'break'.");
}

void Compiler::continue_statement() noexcept {
  if (current_loop_ == nullptr) {
    error("Can't use 'continue' outside of a loop.");
    return;
  }

  // Pop locals deeper than the loop body's scope
  for (int i = local_count_ - 1; i >= 0 && locals_[i].depth > current_loop_->continue_depth; i--) {
    if (locals_[i].is_captured) {
      emit_op(OpCode::OP_CLOSE_UPVALUE);
    } else {
      emit_op(OpCode::OP_POP);
    }
  }

  emit_loop(current_loop_->continue_target);

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
}

void Compiler::return_statement() noexcept {
  if (type_ == FunctionType::TYPE_SCRIPT) {
    error("Can't return from top-level code.");
  }

  if (match(TokenType::TOKEN_SEMICOLON)) {
    emit_return();
  } else {
    if (type_ == FunctionType::TYPE_INITIALIZER) {
      error("Can't return a value from an initializer.");
    }

    expression();
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after return value.");
    emit_op(OpCode::OP_RETURN);
  }
}

void Compiler::try_statement() noexcept {
  // try { body } catch (name) { handler }

  // Emit OP_TRY with placeholder offset to catch block
  sz_t try_jump = emit_jump(OpCode::OP_TRY);

  // Compile try body
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' after 'try'.");
  begin_scope();
  block();
  end_scope();

  // Normal exit: pop handler and jump over catch block
  emit_op(OpCode::OP_END_TRY);
  sz_t end_jump = emit_jump(OpCode::OP_JUMP);

  // Patch try_jump to here (start of catch block)
  patch_jump(try_jump);

  // Parse catch clause
  consume(TokenType::TOKEN_CATCH, "Expect 'catch' after try block.");
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'catch'.");
  consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name.");
  Token error_name = ps_->previous;
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after catch variable.");

  // Catch body: the exception value is on the stack (pushed by VM)
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before catch body.");
  begin_scope();
  add_local(error_name);
  mark_initialized();
  block();
  end_scope();

  // Patch end_jump to here (after catch block)
  patch_jump(end_jump);
}

void Compiler::throw_statement() noexcept {
  expression();
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after throw value.");
  emit_op(OpCode::OP_THROW);
}

void Compiler::defer_statement() noexcept {
  // Wrap the deferred statement in a zero-arg closure:
  //   defer <stmt>;  →  OP_CLOSURE <wrapper> + OP_DEFER
  // The wrapper function contains the compiled statement + implicit return.

  Compiler compiler(*ps_, FunctionType::TYPE_FUNCTION);
  compiler.begin_scope();

  // Compile the deferred statement inside the wrapper function
  compiler.statement();

  ObjFunction* wrapper = compiler.end_compiler();
  wrapper->set_name(VM::get_instance().copy_string("<defer>", 7));

  sz_t closure_index = make_constant(Value(static_cast<Object*>(wrapper)));
  if (closure_index > 255) {
    error("Too many constants in one chunk.");
    return;
  }
  emit_op_byte(OpCode::OP_CLOSURE, static_cast<u8_t>(closure_index));

  // Emit upvalue capture metadata
  for (int i = 0; i < wrapper->upvalue_count(); i++) {
    emit_byte(compiler.upvalues_[i].is_local ? 1 : 0);
    emit_byte(compiler.upvalues_[i].index);
  }

  emit_op(OpCode::OP_DEFER);
}

void Compiler::switch_statement() noexcept {
  // switch (value) { case <expr>: <body> ... default: <body> }
  //
  // Bytecode layout (no fall-through — each case is exclusive):
  //   <switch expr>             → hidden local " switch" (slot N)
  //   GET_LOCAL N, <case1 expr>, OP_EQUAL, JUMP_IF_FALSE -> case2
  //   POP (true), <case1 body>, JUMP -> end
  //   case2: POP (false)
  //   GET_LOCAL N, <case2 expr>, OP_EQUAL, JUMP_IF_FALSE -> case3
  //   ...
  //   default: <default body>
  //   end:
  //   end_scope                 → pops hidden local

  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after value.");
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before switch cases.");

  // Store switch value as a hidden local
  begin_scope();
  Token switch_token{TokenType::TOKEN_IDENTIFIER, " switch", ps_->previous.line};
  add_local(switch_token);
  mark_initialized();
  int switch_slot = local_count_ - 1;

  // Set up break context (reuse LoopContext for break support)
  LoopContext switch_ctx;
  switch_ctx.enclosing = current_loop_;
  switch_ctx.break_depth = scope_depth_;
  current_loop_ = &switch_ctx;

  std::vector<sz_t> case_ends;
  bool had_default = false;

  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    if (match(TokenType::TOKEN_CASE)) {
      // Load switch value and case expression, compare
      emit_op_byte(OpCode::OP_GET_LOCAL, static_cast<u8_t>(switch_slot));
      expression();
      emit_op(OpCode::OP_EQUAL);

      sz_t next_case = emit_jump(OpCode::OP_JUMP_IF_FALSE);
      emit_op(OpCode::OP_POP); // pop true

      consume(TokenType::TOKEN_COLON, "Expect ':' after case value.");

      // Parse case body statements
      while (!check(TokenType::TOKEN_CASE) && !check(TokenType::TOKEN_DEFAULT) &&
             !check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
        declaration();
      }

      case_ends.push_back(emit_jump(OpCode::OP_JUMP)); // jump to end

      patch_jump(next_case);
      emit_op(OpCode::OP_POP); // pop false
    } else if (match(TokenType::TOKEN_DEFAULT)) {
      if (had_default) {
        error("Already has a default case in switch statement.");
      }
      had_default = true;

      consume(TokenType::TOKEN_COLON, "Expect ':' after 'default'.");

      // Parse default body statements
      while (!check(TokenType::TOKEN_CASE) && !check(TokenType::TOKEN_DEFAULT) &&
             !check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
        declaration();
      }
    } else {
      error("Expect 'case' or 'default' in switch statement.");
      advance();
    }
  }

  // Patch all case-end jumps and break jumps to here
  for (sz_t offset : case_ends) {
    patch_jump(offset);
  }

  current_loop_ = switch_ctx.enclosing;

  for (sz_t offset : switch_ctx.break_jumps) {
    patch_jump(offset);
  }

  end_scope(); // pops hidden switch local

  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after switch body.");
}

void Compiler::synchronize() noexcept {
  ps_->panic_mode = false;

  while (ps_->current.type != TokenType::TOKEN_EOF) {
    if (ps_->previous.type == TokenType::TOKEN_SEMICOLON) return;
    switch (ps_->current.type) {
    case TokenType::TOKEN_CLASS:
    case TokenType::TOKEN_FUN:
    case TokenType::TOKEN_VAR:
    case TokenType::TOKEN_FOR:
    case TokenType::TOKEN_IF:
    case TokenType::TOKEN_WHILE:
    case TokenType::TOKEN_PRINT:
    case TokenType::TOKEN_RETURN:
    case TokenType::TOKEN_TRY:
    case TokenType::TOKEN_THROW:
    case TokenType::TOKEN_DEFER:
    case TokenType::TOKEN_SWITCH:
    case TokenType::TOKEN_IMPORT:
    case TokenType::TOKEN_FROM:
      return;
    default:
      ;
    }
    advance();
  }
}

void Compiler::statement() noexcept {
  if (match(TokenType::TOKEN_PRINT)) {
    print_statement();
  } else if (match(TokenType::TOKEN_IF)) {
    if_statement();
  } else if (match(TokenType::TOKEN_RETURN)) {
    return_statement();
  } else if (match(TokenType::TOKEN_WHILE)) {
    while_statement();
  } else if (match(TokenType::TOKEN_FOR)) {
    for_statement();
  } else if (match(TokenType::TOKEN_BREAK)) {
    break_statement();
  } else if (match(TokenType::TOKEN_CONTINUE)) {
    continue_statement();
  } else if (match(TokenType::TOKEN_TRY)) {
    try_statement();
  } else if (match(TokenType::TOKEN_THROW)) {
    throw_statement();
  } else if (match(TokenType::TOKEN_DEFER)) {
    defer_statement();
  } else if (match(TokenType::TOKEN_SWITCH)) {
    switch_statement();
  } else if (match(TokenType::TOKEN_LEFT_BRACE)) {
    begin_scope();
    block();
    end_scope();
  } else {
    expression_statement();
  }
}

void Compiler::declaration() noexcept {
  if (match(TokenType::TOKEN_CLASS)) {
    class_declaration();
  } else if (match(TokenType::TOKEN_FUN)) {
    fun_declaration();
  } else if (match(TokenType::TOKEN_VAR)) {
    var_declaration();
  } else if (match(TokenType::TOKEN_IMPORT)) {
    import_declaration();
  } else if (match(TokenType::TOKEN_FROM)) {
    // from "path" import name [as alias];
    consume(TokenType::TOKEN_STRING, "Expect module path string after 'from'.");
    Value path_value = Value(static_cast<Object*>(
        VM::get_instance().copy_string(ps_->previous.lexeme.data() + 1,
                       ps_->previous.lexeme.length() - 2)));
    consume(TokenType::TOKEN_IMPORT, "Expect 'import' after module path.");
    consume(TokenType::TOKEN_IDENTIFIER, "Expect name to import.");
    Value name_value = Value(static_cast<Object*>(
        VM::get_instance().copy_string(ps_->previous.lexeme.data(), ps_->previous.lexeme.length())));

    if (match(TokenType::TOKEN_AS)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect alias name after 'as'.");
      Value alias_value = Value(static_cast<Object*>(
          VM::get_instance().copy_string(ps_->previous.lexeme.data(), ps_->previous.lexeme.length())));

      emit_constant(path_value);
      emit_constant(name_value);
      emit_constant(alias_value);
      emit_op(OpCode::OP_IMPORT_ALIAS);
    } else {
      emit_constant(path_value);
      emit_constant(name_value);
      emit_op(OpCode::OP_IMPORT_FROM);
    }

    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import statement.");
  } else {
    statement();
  }

  if (ps_->panic_mode) synchronize();
}

ObjFunction* Compiler::end_compiler() noexcept {
  emit_return();
  ObjFunction* function = function_;

#ifdef MAPLE_DEBUG_PRINT
  if (!ps_->had_error) {
    disassemble_chunk(current_chunk(),
        function->name() != nullptr ? strv_t(function->name()->value()) : "<script>");
  }
#endif

  ps_->current_compiler = enclosing_;
  return function;
}

ObjFunction* compile(strv_t source, strv_t script_path) noexcept {
  ParseState ps;
  ps.scanner.init(source);
  ps.script_path = str_t(script_path);
  active_parse_state_ = &ps;

  Compiler compiler(ps, FunctionType::TYPE_SCRIPT);
  compiler.advance();

  while (!compiler.match(TokenType::TOKEN_EOF)) {
    compiler.declaration();
  }

  ObjFunction* function = compiler.end_compiler();
  active_parse_state_ = nullptr;
  return ps.had_error ? nullptr : function;
}

void mark_compiler_roots() noexcept {
  if (active_parse_state_ == nullptr) return;
  Compiler* compiler = active_parse_state_->current_compiler;
  while (compiler != nullptr) {
    mark_object(compiler->function_);
    compiler = compiler->enclosing_;
  }
}

} // namespace ms
