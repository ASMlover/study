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

// =============================================================================
// Precedence levels
// =============================================================================

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

// =============================================================================
// Expression Descriptor — tracks where an expression result lives
// =============================================================================

struct ExprDesc {
  enum Kind : u8_t {
    VNIL,     // nil literal
    VTRUE,    // true literal
    VFALSE,   // false literal
    VK,       // constant; info = constant index
    VLOCAL,   // local variable; info = register slot
    VUPVAL,   // upvalue; info = upvalue index
    VGLOBAL,  // global; info = constant name index
    VREG,     // value in a temporary register; info = register
  };
  Kind kind{VNIL};
  u8_t info{0};
};

// =============================================================================
// Forward declarations and types
// =============================================================================

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

// =============================================================================
// Compiler class (internal to this TU)
// =============================================================================

class Compiler {
  ParseState* ps_;

  Compiler* enclosing_{nullptr};
  ObjFunction* function_{nullptr};
  FunctionType type_;

  Local locals_[kUINT8_COUNT];
  int local_count_{0};
  Upvalue upvalues_[kUINT8_COUNT];
  int scope_depth_{0};
  LoopContext* current_loop_{nullptr};

  // Register allocation
  u8_t reg_top_{0};
  int max_reg_{0};

  // Expression descriptor for the last compiled expression
  ExprDesc last_expr_{};

  // Constant folding
  struct ConstRecord {
    sz_t code_offset{0};
    sz_t const_index{0};
    bool valid{false};
  };
  ConstRecord last_const_;
  ConstRecord prev_const_;

  void record_constant(sz_t code_offset, sz_t const_index) noexcept;
  void invalidate_constants() noexcept;
  bool try_fold_unary(OpCode op) noexcept;
  bool try_fold_binary(OpCode op) noexcept;

  // Register allocation helpers
  u8_t alloc_reg() noexcept;
  void free_reg(u8_t reg) noexcept;
  void check_stack(u8_t reg) noexcept;
  u8_t discharge(ExprDesc& e) noexcept;
  u8_t to_rk(ExprDesc& e) noexcept;
  void expr_to_reg(ExprDesc& e, u8_t target) noexcept;
  void expr_to_nextreg(ExprDesc& e) noexcept;

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
  void emit_instr(Instruction instr) noexcept;
  sz_t emit_jump(OpCode op) noexcept;
  void patch_jump(sz_t offset) noexcept;
  void emit_loop(sz_t loop_start) noexcept;
  void emit_return() noexcept;
  sz_t make_constant(Value value) noexcept;

  // Variables
  u16_t identifier_constant(const Token& name) noexcept;
  u16_t parse_variable(strv_t error_msg) noexcept;
  void define_variable(u16_t global, u8_t value_reg) noexcept;
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
  u8_t argument_list(u8_t base) noexcept;

  friend ObjFunction* compile(strv_t source, strv_t script_path) noexcept;
  friend ObjFunction* compile(strv_t source, strv_t script_path,
                              std::vector<Diagnostic>& diagnostics) noexcept;
  friend void mark_compiler_roots() noexcept;
};

static ParseState* active_parse_state_ = nullptr;

// =============================================================================
// Parse rule table — indexed by TokenType enum value.
// Order MUST match TokenTypes.hh; static_assert below verifies size.
// =============================================================================

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
  // TOKEN_ABSTRACT
  { nullptr,             nullptr,            Precedence::PREC_NONE },
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

// =============================================================================
// Compiler constructor
// =============================================================================

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

  // Slot 0 is reserved: "this" for methods, "" for functions
  Local& local = locals_[local_count_++];
  local.depth = 0;
  local.is_captured = false;
  if (type != FunctionType::TYPE_FUNCTION) {
    local.name.lexeme = "this";
  } else {
    local.name.lexeme = "";
  }

  reg_top_ = static_cast<u8_t>(local_count_);
  max_reg_ = reg_top_;
}

// =============================================================================
// Register allocation
// =============================================================================

inline u8_t Compiler::alloc_reg() noexcept {
  if (reg_top_ >= kMAX_REGISTERS) {
    error("Too many registers needed (function too complex).");
    return 0;
  }
  u8_t r = reg_top_++;
  check_stack(reg_top_);
  return r;
}

inline void Compiler::free_reg(u8_t reg) noexcept {
  if (reg == reg_top_ - 1 && reg >= static_cast<u8_t>(local_count_)) {
    reg_top_--;
  }
}

inline void Compiler::check_stack(u8_t reg) noexcept {
  if (reg > max_reg_) {
    max_reg_ = reg;
  }
}

u8_t Compiler::discharge(ExprDesc& e) noexcept {
  switch (e.kind) {
  case ExprDesc::VLOCAL:
    return e.info; // already in a register
  case ExprDesc::VREG:
    return e.info; // already in a temp register
  case ExprDesc::VK: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(e.info)));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  case ExprDesc::VNIL: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_LOADNIL, dest, 0, 0));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  case ExprDesc::VTRUE: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_LOADTRUE, dest, 0, 0));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  case ExprDesc::VFALSE: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_LOADFALSE, dest, 0, 0));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  case ExprDesc::VUPVAL: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_GETUPVAL, dest, e.info, 0));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  case ExprDesc::VGLOBAL: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, dest, static_cast<u16_t>(e.info)));
    e.kind = ExprDesc::VREG;
    e.info = dest;
    return dest;
  }
  }
  return 0;
}

u8_t Compiler::to_rk(ExprDesc& e) noexcept {
  if (e.kind == ExprDesc::VK && e.info < kMAX_RK_CONST) {
    return const_to_rk(e.info);
  }
  return discharge(e);
}

void Compiler::expr_to_reg(ExprDesc& e, u8_t target) noexcept {
  switch (e.kind) {
  case ExprDesc::VLOCAL:
    if (e.info != target) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, target, e.info, 0));
    }
    break;
  case ExprDesc::VREG:
    if (e.info != target) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, target, e.info, 0));
      free_reg(e.info);
    }
    break;
  case ExprDesc::VK:
    emit_instr(encode_ABx(OpCode::OP_LOADK, target, static_cast<u16_t>(e.info)));
    break;
  case ExprDesc::VNIL:
    emit_instr(encode_ABC(OpCode::OP_LOADNIL, target, 0, 0));
    break;
  case ExprDesc::VTRUE:
    emit_instr(encode_ABC(OpCode::OP_LOADTRUE, target, 0, 0));
    break;
  case ExprDesc::VFALSE:
    emit_instr(encode_ABC(OpCode::OP_LOADFALSE, target, 0, 0));
    break;
  case ExprDesc::VUPVAL:
    emit_instr(encode_ABC(OpCode::OP_GETUPVAL, target, e.info, 0));
    break;
  case ExprDesc::VGLOBAL:
    emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, target, static_cast<u16_t>(e.info)));
    break;
  }
  e.kind = ExprDesc::VREG;
  e.info = target;
}

void Compiler::expr_to_nextreg(ExprDesc& e) noexcept {
  u8_t dest = alloc_reg();
  expr_to_reg(e, dest);
}

// =============================================================================
// Constant folding
// =============================================================================

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

  if (op == OpCode::OP_NEG) {
    if (operand.is_integer()) {
      i64_t result = -operand.as_integer();
      current_chunk().truncate(last_const_.code_offset);
      last_const_.valid = false;
      sz_t code_offset = current_chunk().count();
      sz_t ki = make_constant(Value(result));
      u8_t dest = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
      record_constant(code_offset, ki);
      last_expr_ = {ExprDesc::VREG, dest};
      return true;
    }
    if (operand.is_double()) {
      double result = -operand.as_number();
      current_chunk().truncate(last_const_.code_offset);
      last_const_.valid = false;
      sz_t code_offset = current_chunk().count();
      sz_t ki = make_constant(Value(result));
      u8_t dest = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
      record_constant(code_offset, ki);
      last_expr_ = {ExprDesc::VREG, dest};
      return true;
    }
  }
  return false;
}

bool Compiler::try_fold_binary(OpCode op) noexcept {
  if (!last_const_.valid || !prev_const_.valid) return false;

  const Value& right = current_chunk().constant_at(last_const_.const_index);
  const Value& left = current_chunk().constant_at(prev_const_.const_index);

  // Integer folding
  if (left.is_integer() && right.is_integer()) {
    i64_t a = left.as_integer();
    i64_t b = right.as_integer();
    Value folded;

    switch (op) {
    case OpCode::OP_ADD: folded = Value(static_cast<i64_t>(a + b)); break;
    case OpCode::OP_SUB: folded = Value(static_cast<i64_t>(a - b)); break;
    case OpCode::OP_MUL: folded = Value(static_cast<i64_t>(a * b)); break;
    case OpCode::OP_DIV:
      if (b == 0) return false;
      folded = Value(static_cast<double>(a) / static_cast<double>(b));
      break;
    case OpCode::OP_MOD:
      if (b == 0) return false;
      folded = Value(static_cast<i64_t>(a % b));
      break;
    default: return false;
    }

    current_chunk().truncate(prev_const_.code_offset);
    invalidate_constants();
    sz_t code_offset = current_chunk().count();
    sz_t ki = make_constant(folded);
    // Reuse a register: free previous temps, alloc fresh
    reg_top_ = static_cast<u8_t>(prev_const_.code_offset == 0 ? local_count_ : reg_top_);
    // Actually we need to restore reg_top to before the two operands were loaded
    // Since we truncated, just alloc a new reg
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    record_constant(code_offset, ki);
    last_expr_ = {ExprDesc::VREG, dest};
    return true;
  }

  // Double folding
  if (left.is_number() && right.is_number()) {
    double a = left.as_number();
    double b = right.as_number();
    double result;

    switch (op) {
    case OpCode::OP_ADD: result = a + b; break;
    case OpCode::OP_SUB: result = a - b; break;
    case OpCode::OP_MUL: result = a * b; break;
    case OpCode::OP_DIV:
      if (b == 0.0) return false;
      result = a / b;
      break;
    case OpCode::OP_MOD:
      if (b == 0.0) return false;
      result = std::fmod(a, b);
      break;
    default: return false;
    }

    current_chunk().truncate(prev_const_.code_offset);
    invalidate_constants();
    sz_t code_offset = current_chunk().count();
    sz_t ki = make_constant(Value(result));
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    record_constant(code_offset, ki);
    last_expr_ = {ExprDesc::VREG, dest};
    return true;
  }

  // String concatenation folding
  if (op == OpCode::OP_ADD && left.is_string() && right.is_string()) {
    str_t result = as_string(left)->value() + as_string(right)->value();
    current_chunk().truncate(prev_const_.code_offset);
    invalidate_constants();
    sz_t code_offset = current_chunk().count();
    sz_t ki = make_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(result.data(), result.length()))));
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    record_constant(code_offset, ki);
    last_expr_ = {ExprDesc::VREG, dest};
    return true;
  }

  return false;
}

// =============================================================================
// Error reporting
// =============================================================================

void Compiler::error_at(const Token& token, strv_t message) noexcept {
  if (ps_->panic_mode) return;
  ps_->panic_mode = true;

  if (ps_->diagnostics) {
    Diagnostic d;
    d.line = token.line;
    d.column = token.column;
    d.end_column = token.column + static_cast<int>(token.lexeme.size());
    d.message = str_t(message);
    ps_->diagnostics->push_back(std::move(d));
  } else {
    std::cerr << std::format("[line {}] Error", token.line);
    if (token.type == TokenType::TOKEN_EOF) {
      std::cerr << " at end";
    } else if (token.type != TokenType::TOKEN_ERROR) {
      std::cerr << std::format(" at '{}'", token.lexeme);
    }
    std::cerr << ": " << message << std::endl;
  }
  ps_->had_error = true;
}

void Compiler::error(strv_t message) noexcept {
  error_at(ps_->previous, message);
}

void Compiler::error_at_current(strv_t message) noexcept {
  error_at(ps_->current, message);
}

// =============================================================================
// Token consumption
// =============================================================================

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

// =============================================================================
// Bytecode emission
// =============================================================================

Chunk& Compiler::current_chunk() noexcept {
  return function_->chunk();
}

inline void Compiler::emit_instr(Instruction instr) noexcept {
  current_chunk().write(instr, ps_->previous.line,
      ps_->previous.column, static_cast<int>(ps_->previous.lexeme.size()));
}

sz_t Compiler::emit_jump(OpCode op) noexcept {
  // Emit JMP/TEST+JMP placeholder with sBx = 0
  sz_t offset = current_chunk().count();
  emit_instr(encode_AsBx(op, 0, 0));
  return offset;
}

void Compiler::patch_jump(sz_t offset) noexcept {
  // offset is where the JMP instruction was emitted
  // target is current_chunk().count()
  // sBx = target - (offset + 1), since PC will be at offset+1 after fetching JMP
  int jump = static_cast<int>(current_chunk().count()) - static_cast<int>(offset) - 1;

  if (jump > kSBX_BIAS || jump < -kSBX_BIAS) {
    error("Too much code to jump over.");
  }

  // Rewrite the instruction preserving the opcode
  OpCode op = decode_op(current_chunk().code_at(offset));
  u8_t a = decode_A(current_chunk().code_at(offset));
  current_chunk()[offset] = encode_AsBx(op, a, jump);
}

void Compiler::emit_loop(sz_t loop_start) noexcept {
  sz_t offset = current_chunk().count();
  int jump = static_cast<int>(loop_start) - static_cast<int>(offset) - 1;

  if (jump > kSBX_BIAS || jump < -kSBX_BIAS) {
    error("Loop body too large.");
  }

  emit_instr(encode_AsBx(OpCode::OP_JMP, 0, jump));
}

void Compiler::emit_return() noexcept {
  if (type_ == FunctionType::TYPE_INITIALIZER) {
    // return this (R(0))
    emit_instr(encode_ABC(OpCode::OP_RETURN, 0, 2, 0));
  } else {
    // return nil: LOADNIL into a temp, then RETURN
    u8_t r = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_LOADNIL, r, 0, 0));
    emit_instr(encode_ABC(OpCode::OP_RETURN, r, 2, 0));
    free_reg(r);
  }
}

sz_t Compiler::make_constant(Value value) noexcept {
  sz_t constant = current_chunk().add_constant(value);
  if (constant > 0xFFFF) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return constant;
}

// =============================================================================
// Variables
// =============================================================================

u16_t Compiler::identifier_constant(const Token& name) noexcept {
  sz_t index = make_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(name.lexeme.data(), name.lexeme.length()))));
  return static_cast<u16_t>(index);
}

u16_t Compiler::parse_variable(strv_t error_msg) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, error_msg);

  declare_variable();
  if (scope_depth_ > 0) return 0;

  return identifier_constant(ps_->previous);
}

void Compiler::define_variable(u16_t global, u8_t value_reg) noexcept {
  if (scope_depth_ > 0) {
    mark_initialized();
    // For locals, the value should already be in the local's register slot.
    // If value_reg != local slot, emit a MOVE.
    u8_t local_slot = static_cast<u8_t>(local_count_ - 1);
    if (value_reg != local_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, local_slot, value_reg, 0));
    }
    // Ensure reg_top is at least past all locals
    if (reg_top_ <= local_slot) {
      reg_top_ = local_slot + 1;
      check_stack(reg_top_);
    }
    return;
  }
  emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, value_reg, global));
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

  // Ensure reg_top covers the new local
  if (reg_top_ < static_cast<u8_t>(local_count_)) {
    reg_top_ = static_cast<u8_t>(local_count_);
    check_stack(reg_top_);
  }
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
static OpCode compound_op(TokenType type) noexcept {
  switch (type) {
  case TokenType::TOKEN_PLUS_EQUAL:    return OpCode::OP_ADD;
  case TokenType::TOKEN_MINUS_EQUAL:   return OpCode::OP_SUB;
  case TokenType::TOKEN_STAR_EQUAL:    return OpCode::OP_MUL;
  case TokenType::TOKEN_SLASH_EQUAL:   return OpCode::OP_DIV;
  case TokenType::TOKEN_PERCENT_EQUAL: return OpCode::OP_MOD;
  default: return OpCode::OP_RETURN; // sentinel: not a compound op
  }
}

static bool is_compound_op(TokenType type) noexcept {
  return compound_op(type) != OpCode::OP_RETURN;
}

void Compiler::named_variable(const Token& name, bool can_assign) noexcept {
  int arg = resolve_local(name);
  if (arg != -1) {
    // Local variable
    u8_t slot = static_cast<u8_t>(arg);
    if (can_assign && match(TokenType::TOKEN_EQUAL)) {
      expression();
      u8_t src = discharge(last_expr_);
      if (src != slot) {
        emit_instr(encode_ABC(OpCode::OP_MOVE, slot, src, 0));
      }
      free_reg(src);
      last_expr_ = {ExprDesc::VLOCAL, slot};
    } else if (can_assign && is_compound_op(ps_->current.type)) {
      OpCode op = compound_op(ps_->current.type);
      advance();
      expression();
      u8_t rk_right = to_rk(last_expr_);
      emit_instr(encode_ABC(op, slot, slot, rk_right));
      if (!is_rk_const(rk_right)) free_reg(rk_right);
      last_expr_ = {ExprDesc::VLOCAL, slot};
    } else {
      last_expr_ = {ExprDesc::VLOCAL, slot};
    }
  } else if ((arg = resolve_upvalue(name)) != -1) {
    // Upvalue
    u8_t uv = static_cast<u8_t>(arg);
    if (can_assign && match(TokenType::TOKEN_EQUAL)) {
      expression();
      u8_t src = discharge(last_expr_);
      emit_instr(encode_ABC(OpCode::OP_SETUPVAL, src, uv, 0));
      last_expr_ = {ExprDesc::VREG, src};
    } else if (can_assign && is_compound_op(ps_->current.type)) {
      OpCode op = compound_op(ps_->current.type);
      advance();
      // Get upvalue into temp
      u8_t dest = alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_GETUPVAL, dest, uv, 0));
      expression();
      u8_t rk_right = to_rk(last_expr_);
      emit_instr(encode_ABC(op, dest, dest, rk_right));
      if (!is_rk_const(rk_right)) free_reg(rk_right);
      emit_instr(encode_ABC(OpCode::OP_SETUPVAL, dest, uv, 0));
      last_expr_ = {ExprDesc::VREG, dest};
    } else {
      last_expr_ = {ExprDesc::VUPVAL, uv};
    }
  } else {
    // Global variable
    u16_t ki = identifier_constant(name);
    if (can_assign && match(TokenType::TOKEN_EQUAL)) {
      expression();
      u8_t src = discharge(last_expr_);
      emit_instr(encode_ABx(OpCode::OP_SETGLOBAL, src, ki));
      last_expr_ = {ExprDesc::VREG, src};
    } else if (can_assign && is_compound_op(ps_->current.type)) {
      OpCode op = compound_op(ps_->current.type);
      advance();
      u8_t dest = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, dest, ki));
      expression();
      u8_t rk_right = to_rk(last_expr_);
      emit_instr(encode_ABC(op, dest, dest, rk_right));
      if (!is_rk_const(rk_right)) free_reg(rk_right);
      emit_instr(encode_ABx(OpCode::OP_SETGLOBAL, dest, ki));
      last_expr_ = {ExprDesc::VREG, dest};
    } else {
      last_expr_ = {ExprDesc::VGLOBAL, static_cast<u8_t>(ki)};
      // For globals with large constant indices, we need to discharge immediately
      // since VGLOBAL only holds u8_t info. Store the full index differently.
      // Actually, GETGLOBAL uses ABx format (16-bit Bx), so we need to discharge.
      u8_t dest = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, dest, ki));
      last_expr_ = {ExprDesc::VREG, dest};
    }
  }
  invalidate_constants();
}

// =============================================================================
// Scoping
// =============================================================================

void Compiler::begin_scope() noexcept {
  scope_depth_++;
}

void Compiler::end_scope() noexcept {
  scope_depth_--;

  // Find the first local that needs to be popped
  bool need_close = false;
  int first_close_slot = -1;

  while (local_count_ > 0 &&
         locals_[local_count_ - 1].depth > scope_depth_) {
    if (locals_[local_count_ - 1].is_captured && !need_close) {
      need_close = true;
      first_close_slot = local_count_ - 1;
    }
    local_count_--;
  }

  if (need_close) {
    emit_instr(encode_ABC(OpCode::OP_CLOSE, static_cast<u8_t>(first_close_slot), 0, 0));
  }

  // Reset reg_top to local_count (free all temps and dead locals)
  reg_top_ = static_cast<u8_t>(local_count_);
}

// =============================================================================
// Parsing by precedence
// =============================================================================

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

// =============================================================================
// Expression parsers
// =============================================================================

void Compiler::grouping(bool /*can_assign*/) noexcept {
  // Empty tuple: ()
  if (check(TokenType::TOKEN_RIGHT_PAREN)) {
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after tuple.");
    u8_t base = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_NEWTUPLE, base, 0, 0));
    last_expr_ = {ExprDesc::VREG, base};
    invalidate_constants();
    return;
  }

  expression();

  if (match(TokenType::TOKEN_COMMA)) {
    // Tuple: (expr, ...) or (expr,)
    // First element already in last_expr_
    // We need consecutive registers: base, elem0, elem1, ...
    // Reserve base register, then place elements starting at base+1
    u8_t base = reg_top_;
    alloc_reg(); // reserve base for NEWTUPLE result

    // Move first element to base+1
    u8_t first_reg = alloc_reg();
    expr_to_reg(last_expr_, first_reg);

    u8_t count = 1;
    if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
      do {
        expression();
        u8_t elem_reg = alloc_reg();
        expr_to_reg(last_expr_, elem_reg);
        if (count == 255) {
          error("Can't have more than 255 elements in a tuple literal.");
        }
        count++;
      } while (match(TokenType::TOKEN_COMMA));
    }
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after tuple elements.");
    emit_instr(encode_ABC(OpCode::OP_NEWTUPLE, base, count, 0));
    // Free element registers
    reg_top_ = base + 1;
    last_expr_ = {ExprDesc::VREG, base};
    invalidate_constants();
  } else {
    // Plain grouping: (expr)
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
  }
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
  sz_t ki = make_constant(Value(value));
  sz_t code_offset = current_chunk().count();
  last_expr_ = {ExprDesc::VK, static_cast<u8_t>(ki > 255 ? 0 : ki)};
  // For large constant indices, discharge immediately
  if (ki > 255) {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    last_expr_ = {ExprDesc::VREG, dest};
    record_constant(current_chunk().count() - 1, ki);
  } else {
    last_expr_ = {ExprDesc::VK, static_cast<u8_t>(ki)};
    // Record for folding but don't emit yet (deferred)
    // We need to record the *potential* code offset where this would be emitted
    // For folding to work, we'll record when we actually discharge
  }
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
  sz_t ki = make_constant(Value(value));
  if (ki > 255) {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    last_expr_ = {ExprDesc::VREG, dest};
  } else {
    last_expr_ = {ExprDesc::VK, static_cast<u8_t>(ki)};
  }
}

void Compiler::string(bool /*can_assign*/) noexcept {
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

  sz_t ki = make_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(result.data(), result.length()))));
  if (ki > 255) {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, dest, static_cast<u16_t>(ki)));
    last_expr_ = {ExprDesc::VREG, dest};
  } else {
    last_expr_ = {ExprDesc::VK, static_cast<u8_t>(ki)};
  }
}

void Compiler::string_interpolation(bool /*can_assign*/) noexcept {
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

  auto emit_str_const = [&](const char* src, sz_t len) -> u8_t {
    str_t seg = process_escapes(src, len);
    sz_t ki = make_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(seg.data(), seg.length()))));
    u8_t r = alloc_reg();
    emit_instr(encode_ABx(OpCode::OP_LOADK, r, static_cast<u16_t>(ki)));
    return r;
  };

  // First segment: lexeme starts with '"', strip it
  const char* first_start = ps_->previous.lexeme.data() + 1;
  sz_t first_len = ps_->previous.lexeme.length() - 1;

  u8_t accum = 0;
  bool has_prev = false;

  if (first_len > 0) {
    accum = emit_str_const(first_start, first_len);
    has_prev = true;
  }

  for (;;) {
    // Parse the interpolated expression
    expression();
    u8_t expr_reg = discharge(last_expr_);

    // Convert to string: STR R(dest), R(expr_reg)
    u8_t str_reg = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_STR, str_reg, expr_reg, 0));
    free_reg(expr_reg);

    if (has_prev) {
      // Concatenate: ADD R(accum), R(accum), R(str_reg)
      emit_instr(encode_ABC(OpCode::OP_ADD, accum, accum, str_reg));
      free_reg(str_reg);
    } else {
      accum = str_reg;
    }
    has_prev = true;

    if (match(TokenType::TOKEN_STRING_INTERP)) {
      const char* seg_start = ps_->previous.lexeme.data();
      sz_t seg_len = ps_->previous.lexeme.length();
      if (seg_len > 0) {
        u8_t seg_reg = emit_str_const(seg_start, seg_len);
        emit_instr(encode_ABC(OpCode::OP_ADD, accum, accum, seg_reg));
        free_reg(seg_reg);
      }
    } else if (match(TokenType::TOKEN_STRING)) {
      const char* seg_start = ps_->previous.lexeme.data();
      sz_t seg_len = ps_->previous.lexeme.length();
      if (seg_len > 0) {
        u8_t seg_reg = emit_str_const(seg_start, seg_len);
        emit_instr(encode_ABC(OpCode::OP_ADD, accum, accum, seg_reg));
        free_reg(seg_reg);
      }
      break;
    } else {
      error("Unterminated string interpolation.");
      break;
    }
  }

  last_expr_ = {ExprDesc::VREG, accum};
  invalidate_constants();
}

void Compiler::variable(bool can_assign) noexcept {
  named_variable(ps_->previous, can_assign);
}

void Compiler::unary(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;

  parse_precedence(Precedence::PREC_UNARY);

  u8_t operand = discharge(last_expr_);

  switch (operator_type) {
  case TokenType::TOKEN_BANG: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_NOT, dest, operand, 0));
    free_reg(operand);
    last_expr_ = {ExprDesc::VREG, dest};
    invalidate_constants();
    break;
  }
  case TokenType::TOKEN_MINUS: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_NEG, dest, operand, 0));
    free_reg(operand);
    last_expr_ = {ExprDesc::VREG, dest};
    invalidate_constants();
    break;
  }
  case TokenType::TOKEN_TILDE: {
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_BNOT, dest, operand, 0));
    free_reg(operand);
    last_expr_ = {ExprDesc::VREG, dest};
    invalidate_constants();
    break;
  }
  default: return;
  }
}

void Compiler::binary(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;
  const ParseRule& rule = get_rule(operator_type);

  // Left operand is in last_expr_
  ExprDesc left_expr = last_expr_;

  parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

  // Right operand is in last_expr_
  ExprDesc right_expr = last_expr_;

  // Get RK encodings for left and right
  u8_t rk_left = to_rk(left_expr);
  u8_t rk_right = to_rk(right_expr);

  u8_t dest = alloc_reg();

  switch (operator_type) {
  case TokenType::TOKEN_PLUS:
    emit_instr(encode_ABC(OpCode::OP_ADD, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_MINUS:
    emit_instr(encode_ABC(OpCode::OP_SUB, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_STAR:
    emit_instr(encode_ABC(OpCode::OP_MUL, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_SLASH:
    emit_instr(encode_ABC(OpCode::OP_DIV, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_PERCENT:
    emit_instr(encode_ABC(OpCode::OP_MOD, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_EQUAL_EQUAL:
    emit_instr(encode_ABC(OpCode::OP_EQ, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_BANG_EQUAL: {
    emit_instr(encode_ABC(OpCode::OP_EQ, dest, rk_left, rk_right));
    emit_instr(encode_ABC(OpCode::OP_NOT, dest, dest, 0));
    break;
  }
  case TokenType::TOKEN_LESS:
    emit_instr(encode_ABC(OpCode::OP_LT, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_LESS_EQUAL:
    emit_instr(encode_ABC(OpCode::OP_LE, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_GREATER:
    // a > b  <==>  b < a
    emit_instr(encode_ABC(OpCode::OP_LT, dest, rk_right, rk_left));
    break;
  case TokenType::TOKEN_GREATER_EQUAL:
    // a >= b  <==>  b <= a
    emit_instr(encode_ABC(OpCode::OP_LE, dest, rk_right, rk_left));
    break;
  default:
    break;
  }

  // Free source registers if they were temps
  if (!is_rk_const(rk_left) && left_expr.kind == ExprDesc::VREG) free_reg(rk_left);
  if (!is_rk_const(rk_right) && right_expr.kind == ExprDesc::VREG) free_reg(rk_right);

  last_expr_ = {ExprDesc::VREG, dest};
  invalidate_constants();
}

void Compiler::bitwise(bool /*can_assign*/) noexcept {
  TokenType operator_type = ps_->previous.type;
  const ParseRule& rule = get_rule(operator_type);

  ExprDesc left_expr = last_expr_;

  parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

  ExprDesc right_expr = last_expr_;

  u8_t rk_left = to_rk(left_expr);
  u8_t rk_right = to_rk(right_expr);

  u8_t dest = alloc_reg();

  switch (operator_type) {
  case TokenType::TOKEN_AMPERSAND:
    emit_instr(encode_ABC(OpCode::OP_BAND, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_PIPE:
    emit_instr(encode_ABC(OpCode::OP_BOR, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_CARET:
    emit_instr(encode_ABC(OpCode::OP_BXOR, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_LEFT_SHIFT:
    emit_instr(encode_ABC(OpCode::OP_SHL, dest, rk_left, rk_right));
    break;
  case TokenType::TOKEN_RIGHT_SHIFT:
    emit_instr(encode_ABC(OpCode::OP_SHR, dest, rk_left, rk_right));
    break;
  default: break;
  }

  if (!is_rk_const(rk_left) && left_expr.kind == ExprDesc::VREG) free_reg(rk_left);
  if (!is_rk_const(rk_right) && right_expr.kind == ExprDesc::VREG) free_reg(rk_right);

  last_expr_ = {ExprDesc::VREG, dest};
  invalidate_constants();
}

void Compiler::literal(bool /*can_assign*/) noexcept {
  switch (ps_->previous.type) {
  case TokenType::TOKEN_FALSE:
    last_expr_ = {ExprDesc::VFALSE, 0};
    break;
  case TokenType::TOKEN_NIL:
    last_expr_ = {ExprDesc::VNIL, 0};
    break;
  case TokenType::TOKEN_TRUE:
    last_expr_ = {ExprDesc::VTRUE, 0};
    break;
  default: return;
  }
  invalidate_constants();
}

void Compiler::and_(bool /*can_assign*/) noexcept {
  // Left operand is in last_expr_. Discharge to a register.
  u8_t left_reg = discharge(last_expr_);

  // If left is false, short-circuit: result = left, skip right branch
  // TESTSET R(dest), R(left), 0  — if bool(R(left)) == false, R(dest)=R(left), else PC++
  // Actually for and: if falsy, keep left; if truthy, evaluate right.
  // TEST R(left), 0 — if bool(R(left)) != 0 (i.e. falsy), skip next instr
  //   means: if R(left) is falsy, skip the JMP (i.e. fall through to use left)
  // Wait, TEST semantics: if (bool(R(A)) != C) then PC++
  //   TEST R(left), 1  → if (bool(R(left)) != 1) skip JMP → if falsy, skip JMP
  //   That means if falsy we skip the JMP and fall through (left is result)
  //   If truthy, we execute JMP to right branch
  // We want: if left is falsy, result = left (don't evaluate right)
  //          if left is truthy, evaluate right, result = right

  // Emit: TEST R(left), 0 — if bool(R(left)) != 0 (truthy), skip next
  //   If truthy: skip JMP, fall through to evaluate right
  //   If falsy: execute JMP to end (result = left)
  emit_instr(encode_ABC(OpCode::OP_TEST, left_reg, 0, 0));
  sz_t end_jump = emit_jump(OpCode::OP_JMP);

  // Truthy: evaluate right side into left_reg
  parse_precedence(Precedence::PREC_AND);
  u8_t right_reg = discharge(last_expr_);
  if (right_reg != left_reg) {
    emit_instr(encode_ABC(OpCode::OP_MOVE, left_reg, right_reg, 0));
    free_reg(right_reg);
  }

  patch_jump(end_jump);
  last_expr_ = {ExprDesc::VREG, left_reg};
  invalidate_constants();
}

void Compiler::or_(bool /*can_assign*/) noexcept {
  // If left is truthy, result = left (don't evaluate right)
  // If left is falsy, evaluate right, result = right
  u8_t left_reg = discharge(last_expr_);

  // TEST R(left), 1 — if bool(R(left)) != 1 (falsy), skip next
  //   If falsy: skip JMP, fall through to evaluate right
  //   If truthy: execute JMP to end (result = left)
  emit_instr(encode_ABC(OpCode::OP_TEST, left_reg, 0, 1));
  sz_t end_jump = emit_jump(OpCode::OP_JMP);

  // Falsy: evaluate right side into left_reg
  parse_precedence(Precedence::PREC_OR);
  u8_t right_reg = discharge(last_expr_);
  if (right_reg != left_reg) {
    emit_instr(encode_ABC(OpCode::OP_MOVE, left_reg, right_reg, 0));
    free_reg(right_reg);
  }

  patch_jump(end_jump);
  last_expr_ = {ExprDesc::VREG, left_reg};
  invalidate_constants();
}

void Compiler::ternary(bool /*can_assign*/) noexcept {
  // Condition is in last_expr_
  u8_t cond_reg = discharge(last_expr_);

  // TEST R(cond), 0 — if falsy, skip next (the JMP to else)
  //   If truthy: skip JMP, fall through to then
  //   If falsy: execute JMP to else
  emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
  sz_t else_jump = emit_jump(OpCode::OP_JMP);

  free_reg(cond_reg);

  // Then branch
  u8_t result_reg = alloc_reg();
  parse_precedence(Precedence::PREC_TERNARY);
  expr_to_reg(last_expr_, result_reg);

  sz_t end_jump = emit_jump(OpCode::OP_JMP);

  patch_jump(else_jump);

  consume(TokenType::TOKEN_COLON, "Expect ':' after ternary then branch.");

  // Else branch
  parse_precedence(Precedence::PREC_TERNARY);
  expr_to_reg(last_expr_, result_reg);

  patch_jump(end_jump);

  last_expr_ = {ExprDesc::VREG, result_reg};
  invalidate_constants();
}

u8_t Compiler::argument_list(u8_t base) noexcept {
  u8_t arg_count = 0;
  if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      u8_t arg_reg = static_cast<u8_t>(base + 1 + arg_count);
      if (reg_top_ <= arg_reg) {
        reg_top_ = arg_reg;
        alloc_reg(); // ensure the slot exists
      } else {
        // Slot already allocated
      }
      expr_to_reg(last_expr_, arg_reg);
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
  // The callee is in last_expr_. We need to place it at a base register,
  // followed by arguments in consecutive registers.
  u8_t base = reg_top_;
  alloc_reg(); // reserve base for callee
  expr_to_reg(last_expr_, base);

  u8_t arg_count = argument_list(base);

  // CALL base, B=arg_count+1, C=2 (single return)
  emit_instr(encode_ABC(OpCode::OP_CALL, base, arg_count + 1, 2));

  // After call, result is in R(base), free everything above
  reg_top_ = base + 1;
  last_expr_ = {ExprDesc::VREG, base};
  invalidate_constants();
}

void Compiler::dot(bool can_assign) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect property name after '.'.");
  u16_t name_ki = identifier_constant(ps_->previous);

  // Object is in last_expr_
  u8_t obj_reg = discharge(last_expr_);

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    // SET_PROPERTY: R(obj).K(name) := R(value)
    expression();
    u8_t val_reg = discharge(last_expr_);
    // SETPROP: R(A).K(B) := R(C)
    emit_instr(encode_ABC(OpCode::OP_SETPROP, obj_reg, static_cast<u8_t>(name_ki), val_reg));
    // EXTRAARG for IC slot
    sz_t ic_slot = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    free_reg(val_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else if (match(TokenType::TOKEN_LEFT_PAREN)) {
    // Method invocation: obj.method(args)
    // INVOKE: A=base, B=argc, C=name_K
    u8_t base = obj_reg;
    // If receiver is a local variable, copy to temp to avoid clobbering
    // (OP_RETURN writes result to R(base), which would overwrite the local)
    if (base < static_cast<u8_t>(local_count_)) {
      base = alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_MOVE, base, obj_reg, 0));
    }
    u8_t arg_count = argument_list(base);
    emit_instr(encode_ABC(OpCode::OP_INVOKE, base, arg_count, static_cast<u8_t>(name_ki)));
    sz_t ic_slot = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    // Result in R(base)
    reg_top_ = base + 1;
    last_expr_ = {ExprDesc::VREG, base};
  } else if (can_assign && is_compound_op(ps_->current.type)) {
    // Compound assignment on property: obj.prop += expr
    OpCode op = compound_op(ps_->current.type);
    advance();

    // Get property into a temp
    u8_t prop_reg = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_GETPROP, prop_reg, obj_reg, static_cast<u8_t>(name_ki)));
    sz_t ic_slot_get = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot_get)));

    expression();
    u8_t rk_right = to_rk(last_expr_);
    emit_instr(encode_ABC(op, prop_reg, prop_reg, rk_right));
    if (!is_rk_const(rk_right)) free_reg(rk_right);

    // Set property
    emit_instr(encode_ABC(OpCode::OP_SETPROP, obj_reg, static_cast<u8_t>(name_ki), prop_reg));
    sz_t ic_slot_set = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot_set)));

    free_reg(prop_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else {
    // GET_PROPERTY: R(A) := R(B).K(C)
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_GETPROP, dest, obj_reg, static_cast<u8_t>(name_ki)));
    sz_t ic_slot = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    free_reg(obj_reg);
    last_expr_ = {ExprDesc::VREG, dest};
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
  u16_t name_ki = identifier_constant(ps_->previous);

  // Load "this" (R(0) in methods)
  Token this_token{TokenType::TOKEN_THIS, "this", ps_->previous.line};
  ExprDesc this_expr;
  int this_slot = resolve_local(this_token);
  if (this_slot != -1) {
    this_expr = {ExprDesc::VLOCAL, static_cast<u8_t>(this_slot)};
  } else {
    int this_uv = resolve_upvalue(this_token);
    this_expr = {ExprDesc::VUPVAL, static_cast<u8_t>(this_uv)};
  }
  u8_t this_reg = discharge(this_expr);

  // Load "super"
  Token super_token{TokenType::TOKEN_SUPER, "super", ps_->previous.line};
  ExprDesc super_expr;
  int super_slot = resolve_local(super_token);
  if (super_slot != -1) {
    super_expr = {ExprDesc::VLOCAL, static_cast<u8_t>(super_slot)};
  } else {
    int super_uv = resolve_upvalue(super_token);
    super_expr = {ExprDesc::VUPVAL, static_cast<u8_t>(super_uv)};
  }

  if (match(TokenType::TOKEN_LEFT_PAREN)) {
    // super.method(args) → SUPERINV
    u8_t base = this_reg;
    // Ensure this is at base
    if (base != reg_top_ - 1) {
      u8_t new_base = reg_top_;
      alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_MOVE, new_base, this_reg, 0));
      if (this_expr.kind == ExprDesc::VREG) free_reg(this_reg);
      base = new_base;
    }

    u8_t arg_count = argument_list(base);

    u8_t super_reg = discharge(super_expr);

    // SUPERINV: A=base, B=argc, C=name_K
    emit_instr(encode_ABC(OpCode::OP_SUPERINV, base, arg_count, static_cast<u8_t>(name_ki)));
    free_reg(super_reg);
    reg_top_ = base + 1;
    last_expr_ = {ExprDesc::VREG, base};
  } else {
    // super.method → GETSUPER
    u8_t super_reg = discharge(super_expr);
    u8_t dest = alloc_reg();
    // GETSUPER R(A) := super(R(B)).K(C)
    emit_instr(encode_ABC(OpCode::OP_GETSUPER, dest, super_reg, static_cast<u8_t>(name_ki)));
    free_reg(super_reg);
    if (this_expr.kind == ExprDesc::VREG) free_reg(this_reg);
    last_expr_ = {ExprDesc::VREG, dest};
  }
  invalidate_constants();
}

void Compiler::fun_expression([[maybe_unused]] bool can_assign) noexcept {
  ps_->previous = Token{TokenType::TOKEN_IDENTIFIER, "<lambda>", ps_->previous.line};
  function(FunctionType::TYPE_FUNCTION);
  invalidate_constants();
}

void Compiler::list_(bool /*can_assign*/) noexcept {
  if (check(TokenType::TOKEN_RIGHT_BRACKET)) {
    consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");
    u8_t base = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_NEWLIST, base, 0, 0));
    last_expr_ = {ExprDesc::VREG, base};
    invalidate_constants();
    return;
  }

  // Save state before parsing first expression to detect comprehension
  auto scanner_state = ps_->scanner.save_state();
  Token saved_current = ps_->current;
  Token saved_previous = ps_->previous;
  sz_t saved_code_count = current_chunk().count();
  u8_t saved_reg_top = reg_top_;
  ExprDesc saved_last_expr = last_expr_;

  expression();

  if (match(TokenType::TOKEN_FOR)) {
    // List comprehension: rewind
    current_chunk().truncate(saved_code_count);
    ps_->scanner.restore_state(scanner_state);
    ps_->current = saved_current;
    ps_->previous = saved_previous;
    reg_top_ = saved_reg_top;
    last_expr_ = saved_last_expr;

    list_comprehension_();
    return;
  }

  // Normal list literal
  // We need consecutive registers: base, elem0, elem1, ...
  u8_t base = saved_reg_top;
  // Set reg_top back, we'll manually place elements
  reg_top_ = saved_reg_top;
  alloc_reg(); // base for NEWLIST result

  // First element
  u8_t elem_reg = alloc_reg();
  expr_to_reg(last_expr_, elem_reg);
  u8_t count = 1;

  while (match(TokenType::TOKEN_COMMA)) {
    expression();
    u8_t er = alloc_reg();
    expr_to_reg(last_expr_, er);
    if (count == 255) {
      error("Can't have more than 255 elements in a list literal.");
    }
    count++;
  }
  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");

  emit_instr(encode_ABC(OpCode::OP_NEWLIST, base, count, 0));
  reg_top_ = base + 1;
  last_expr_ = {ExprDesc::VREG, base};
  invalidate_constants();
}

void Compiler::list_comprehension_() noexcept {
  // [expr for var in iterable]
  // [expr for var in iterable if condition]

  // Sync register top with locals so hidden locals align with registers
  reg_top_ = static_cast<u8_t>(local_count_);
  begin_scope();

  // Create empty list as hidden local
  u8_t list_slot = static_cast<u8_t>(local_count_);
  {
    u8_t r = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_NEWLIST, r, 0, 0));
    Token list_token{TokenType::TOKEN_IDENTIFIER, " __list", ps_->previous.line};
    add_local(list_token);
    mark_initialized();
  }

  // Save scanner state for the element expression
  auto expr_scanner_state = ps_->scanner.save_state();
  Token expr_current = ps_->current;
  Token expr_previous = ps_->previous;

  // Parse and discard the expression to advance past it
  {
    sz_t discard_start = current_chunk().count();
    u8_t discard_reg = reg_top_;
    expression();
    current_chunk().truncate(discard_start);
    reg_top_ = discard_reg;
  }

  // Now at "for"
  consume(TokenType::TOKEN_FOR, "Expect 'for' in list comprehension.");
  Token var_name = ps_->current;
  consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name after 'for'.");
  consume(TokenType::TOKEN_IN, "Expect 'in' after variable name.");

  // Compile iterable expression → hidden local __seq
  expression();
  {
    u8_t seq_reg = discharge(last_expr_);
    u8_t seq_slot = static_cast<u8_t>(local_count_);
    if (seq_reg != seq_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, seq_slot, seq_reg, 0));
      if (seq_reg >= static_cast<u8_t>(local_count_)) free_reg(seq_reg);
    }
    Token seq_token{TokenType::TOKEN_IDENTIFIER, " __seq", ps_->previous.line};
    add_local(seq_token);
    mark_initialized();
  }
  u8_t seq_slot = static_cast<u8_t>(local_count_ - 1);

  // Create hidden local __idx = 0
  {
    sz_t ki = make_constant(Value(static_cast<i64_t>(0)));
    u8_t idx_slot = static_cast<u8_t>(local_count_);
    if (reg_top_ <= idx_slot) {
      reg_top_ = idx_slot;
      alloc_reg();
    }
    emit_instr(encode_ABx(OpCode::OP_LOADK, idx_slot, static_cast<u16_t>(ki)));
    Token idx_token{TokenType::TOKEN_IDENTIFIER, " __idx", ps_->previous.line};
    add_local(idx_token);
    mark_initialized();
  }

  // Check for optional "if"
  bool has_filter = false;
  ScannerState filter_scanner_state{};
  Token filter_current{};
  Token filter_previous{};

  if (match(TokenType::TOKEN_IF)) {
    has_filter = true;
    filter_scanner_state = ps_->scanner.save_state();
    filter_current = ps_->current;
    filter_previous = ps_->previous;

    // Skip condition
    sz_t discard_start = current_chunk().count();
    u8_t discard_reg = reg_top_;
    expression();
    current_chunk().truncate(discard_start);
    reg_top_ = discard_reg;
  }

  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after list comprehension.");

  auto after_scanner_state = ps_->scanner.save_state();
  Token after_current = ps_->current;
  Token after_previous = ps_->previous;

  // Loop start
  sz_t loop_start = current_chunk().count();

  // FORITER: seq=R(A), idx=R(A+1), elem→R(A+2); sBx=exit offset
  sz_t foriter_offset = current_chunk().count();
  emit_instr(encode_AsBx(OpCode::OP_FORITER, seq_slot, 0)); // placeholder

  // Inner scope for loop variable
  begin_scope();
  {
    // Element is in R(seq_slot + 2) after FORITER
    Token elem_tok = var_name;
    add_local(elem_tok);
    mark_initialized();
  }

  if (has_filter) {
    // Compile filter condition
    ps_->scanner.restore_state(filter_scanner_state);
    ps_->current = filter_current;
    ps_->previous = filter_previous;
    expression();
    u8_t cond_reg = discharge(last_expr_);

    // TEST R(cond), 0 — if truthy, skip JMP (continue to push)
    emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
    sz_t skip_jump = emit_jump(OpCode::OP_JMP);
    free_reg(cond_reg);

    // Push to list: invoke list.push(elem)
    {
      u8_t base = reg_top_;
      alloc_reg(); // for list ref (callee)
      emit_instr(encode_ABC(OpCode::OP_MOVE, base, list_slot, 0));

      // Compile element expression
      ps_->scanner.restore_state(expr_scanner_state);
      ps_->current = expr_current;
      ps_->previous = expr_previous;
      expression();
      u8_t arg_reg = static_cast<u8_t>(base + 1);
      if (reg_top_ <= arg_reg) { reg_top_ = arg_reg; alloc_reg(); }
      expr_to_reg(last_expr_, arg_reg);

      u16_t push_name = identifier_constant(Token{TokenType::TOKEN_IDENTIFIER, "push", ps_->previous.line});
      emit_instr(encode_ABC(OpCode::OP_INVOKE, base, 1, static_cast<u8_t>(push_name)));
      sz_t ic_slot = function_->add_ic();
      emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
      reg_top_ = base;
    }

    sz_t end_jump = emit_jump(OpCode::OP_JMP);
    patch_jump(skip_jump);
    // Falsy: skip push
    patch_jump(end_jump);
  } else {
    // No filter: push element
    u8_t base = reg_top_;
    alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_MOVE, base, list_slot, 0));

    ps_->scanner.restore_state(expr_scanner_state);
    ps_->current = expr_current;
    ps_->previous = expr_previous;
    expression();
    u8_t arg_reg = static_cast<u8_t>(base + 1);
    if (reg_top_ <= arg_reg) { reg_top_ = arg_reg; alloc_reg(); }
    expr_to_reg(last_expr_, arg_reg);

    u16_t push_name = identifier_constant(Token{TokenType::TOKEN_IDENTIFIER, "push", ps_->previous.line});
    emit_instr(encode_ABC(OpCode::OP_INVOKE, base, 1, static_cast<u8_t>(push_name)));
    sz_t ic_slot = function_->add_ic();
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    reg_top_ = base;
  }

  end_scope(); // pops loop variable

  emit_loop(loop_start);

  // Patch FORITER exit jump
  int exit_offset = static_cast<int>(current_chunk().count()) - static_cast<int>(foriter_offset) - 1;
  current_chunk()[foriter_offset] = encode_AsBx(OpCode::OP_FORITER, seq_slot, exit_offset);

  // Result = list
  last_expr_ = {ExprDesc::VLOCAL, list_slot};
  // Discharge to a temp before ending scope
  u8_t result = alloc_reg();
  emit_instr(encode_ABC(OpCode::OP_MOVE, result, list_slot, 0));

  end_scope(); // pops __idx, __seq, __list

  last_expr_ = {ExprDesc::VREG, result};

  ps_->scanner.restore_state(after_scanner_state);
  ps_->current = after_current;
  ps_->previous = after_previous;

  invalidate_constants();
}

void Compiler::map_(bool /*can_assign*/) noexcept {
  // {key: value, ...}
  u8_t base = reg_top_;
  alloc_reg(); // reserve base for NEWMAP result

  u8_t count = 0;
  if (!check(TokenType::TOKEN_RIGHT_BRACE)) {
    do {
      // Key
      expression();
      u8_t key_reg = alloc_reg();
      expr_to_reg(last_expr_, key_reg);
      consume(TokenType::TOKEN_COLON, "Expect ':' after map key.");
      // Value
      expression();
      u8_t val_reg = alloc_reg();
      expr_to_reg(last_expr_, val_reg);
      if (count == 255) {
        error("Can't have more than 255 entries in a map literal.");
      }
      count++;
    } while (match(TokenType::TOKEN_COMMA));
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after map entries.");
  emit_instr(encode_ABC(OpCode::OP_NEWMAP, base, count, 0));
  reg_top_ = base + 1;
  last_expr_ = {ExprDesc::VREG, base};
  invalidate_constants();
}

void Compiler::subscript_(bool can_assign) noexcept {
  // Object is in last_expr_
  u8_t obj_reg = discharge(last_expr_);

  expression();
  u8_t idx_reg = discharge(last_expr_);
  consume(TokenType::TOKEN_RIGHT_BRACKET, "Expect ']' after index.");

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    u8_t val_reg = discharge(last_expr_);
    // SETIDX R(A)[R(B)] := R(C)
    emit_instr(encode_ABC(OpCode::OP_SETIDX, obj_reg, idx_reg, val_reg));
    free_reg(val_reg);
    free_reg(idx_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else if (can_assign && is_compound_op(ps_->current.type)) {
    OpCode op = compound_op(ps_->current.type);
    advance();
    // Get index value
    u8_t cur_val = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_GETIDX, cur_val, obj_reg, idx_reg));
    expression();
    u8_t rk_right = to_rk(last_expr_);
    emit_instr(encode_ABC(op, cur_val, cur_val, rk_right));
    if (!is_rk_const(rk_right)) free_reg(rk_right);
    emit_instr(encode_ABC(OpCode::OP_SETIDX, obj_reg, idx_reg, cur_val));
    free_reg(cur_val);
    free_reg(idx_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else {
    // GETIDX R(A) := R(B)[R(C)]
    u8_t dest = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_GETIDX, dest, obj_reg, idx_reg));
    free_reg(idx_reg);
    free_reg(obj_reg);
    last_expr_ = {ExprDesc::VREG, dest};
  }
  invalidate_constants();
}

// =============================================================================
// Statements and declarations
// =============================================================================

void Compiler::block() noexcept {
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    declaration();
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::var_declaration() noexcept {
  // Destructuring: var (x, y, z) = expr;
  if (match(TokenType::TOKEN_LEFT_PAREN)) {
    std::vector<Token> names;
    do {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name in destructuring.");
      names.push_back(ps_->previous);
    } while (match(TokenType::TOKEN_COMMA));
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after destructuring variables.");
    consume(TokenType::TOKEN_EQUAL, "Expect '=' after destructuring pattern.");

    expression();
    u8_t tuple_reg = discharge(last_expr_);

    if (scope_depth_ > 0) {
      // Local scope: create a hidden local for the tuple, then index into it
      Token tmp_token{TokenType::TOKEN_IDENTIFIER, " __destruct", ps_->previous.line};
      add_local(tmp_token);
      mark_initialized();
      u8_t tmp_slot = static_cast<u8_t>(local_count_ - 1);
      if (tuple_reg != tmp_slot) {
        emit_instr(encode_ABC(OpCode::OP_MOVE, tmp_slot, tuple_reg, 0));
        if (tuple_reg >= static_cast<u8_t>(local_count_)) free_reg(tuple_reg);
      }

      for (sz_t i = 0; i < names.size(); i++) {
        u8_t local_slot = static_cast<u8_t>(local_count_);
        if (reg_top_ <= local_slot) { reg_top_ = local_slot; alloc_reg(); }
        sz_t idx_ki = make_constant(Value(static_cast<i64_t>(i)));
        u8_t idx_r = alloc_reg();
        emit_instr(encode_ABx(OpCode::OP_LOADK, idx_r, static_cast<u16_t>(idx_ki)));
        emit_instr(encode_ABC(OpCode::OP_GETIDX, local_slot, tmp_slot, idx_r));
        free_reg(idx_r);
        add_local(names[i]);
        mark_initialized();
      }
    } else {
      // Global scope
      begin_scope();
      Token tmp_token{TokenType::TOKEN_IDENTIFIER, " __destruct", ps_->previous.line};
      add_local(tmp_token);
      mark_initialized();
      u8_t tmp_slot = static_cast<u8_t>(local_count_ - 1);
      if (tuple_reg != tmp_slot) {
        emit_instr(encode_ABC(OpCode::OP_MOVE, tmp_slot, tuple_reg, 0));
        if (tuple_reg >= static_cast<u8_t>(local_count_)) free_reg(tuple_reg);
      }

      for (sz_t i = 0; i < names.size(); i++) {
        sz_t idx_ki = make_constant(Value(static_cast<i64_t>(i)));
        u8_t val_reg = alloc_reg();
        u8_t idx_r = alloc_reg();
        emit_instr(encode_ABx(OpCode::OP_LOADK, idx_r, static_cast<u16_t>(idx_ki)));
        emit_instr(encode_ABC(OpCode::OP_GETIDX, val_reg, tmp_slot, idx_r));
        free_reg(idx_r);
        u16_t global = identifier_constant(names[i]);
        emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, val_reg, global));
        free_reg(val_reg);
      }

      end_scope();
    }

    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    invalidate_constants();
    return;
  }

  u16_t global = parse_variable("Expect variable name.");

  if (scope_depth_ > 0) {
    // Local variable: compile initializer directly into the local's slot
    u8_t local_slot = static_cast<u8_t>(local_count_ - 1);
    if (match(TokenType::TOKEN_EQUAL)) {
      expression();
      expr_to_reg(last_expr_, local_slot);
    } else {
      emit_instr(encode_ABC(OpCode::OP_LOADNIL, local_slot, 0, 0));
    }
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    mark_initialized();
    if (reg_top_ <= local_slot) {
      reg_top_ = local_slot + 1;
      check_stack(reg_top_);
    }
  } else {
    // Global variable
    u8_t val_reg;
    if (match(TokenType::TOKEN_EQUAL)) {
      expression();
      val_reg = discharge(last_expr_);
    } else {
      val_reg = alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_LOADNIL, val_reg, 0, 0));
    }
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, val_reg, global));
    free_reg(val_reg);
  }
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
      u16_t constant = compiler.parse_variable("Expect parameter name.");
      // For local params, they are already in the right register slot
      compiler.mark_initialized();
      // Ensure reg_top covers params
      if (compiler.reg_top_ < static_cast<u8_t>(compiler.local_count_)) {
        compiler.reg_top_ = static_cast<u8_t>(compiler.local_count_);
        compiler.check_stack(compiler.reg_top_);
      }
      (void)constant;
    } while (match(TokenType::TOKEN_COMMA));
  }
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  compiler.block();

  ObjFunction* fn = compiler.end_compiler();
  sz_t closure_index = make_constant(Value(static_cast<Object*>(fn)));

  u8_t dest = alloc_reg();
  emit_instr(encode_ABx(OpCode::OP_CLOSURE, dest, static_cast<u16_t>(closure_index)));

  // Emit EXTRAARG for each upvalue
  for (int i = 0; i < fn->upvalue_count(); i++) {
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG,
        compiler.upvalues_[i].is_local ? 1 : 0,
        static_cast<u16_t>(compiler.upvalues_[i].index)));
  }

  last_expr_ = {ExprDesc::VREG, dest};
}

void Compiler::fun_declaration() noexcept {
  u16_t global = parse_variable("Expect function name.");
  mark_initialized();
  function(FunctionType::TYPE_FUNCTION);

  if (scope_depth_ > 0) {
    // Local function
    u8_t local_slot = static_cast<u8_t>(local_count_ - 1);
    u8_t src = discharge(last_expr_);
    if (src != local_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, local_slot, src, 0));
      free_reg(src);
    }
  } else {
    // Global function
    u8_t src = discharge(last_expr_);
    emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, src, global));
    free_reg(src);
  }
}

void Compiler::method() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect method name.");
  u16_t constant = identifier_constant(ps_->previous);

  FunctionType type = FunctionType::TYPE_METHOD;
  if (ps_->previous.lexeme == "init") {
    type = FunctionType::TYPE_INITIALIZER;
  }
  function(type);

  u8_t closure_reg = discharge(last_expr_);
  // class is in the register below (loaded by class_declaration)
  // We need to reference the class register. The class_declaration sets it up.
  // METHOD R(class), K(name), R(closure)
  // The class register is managed by class_declaration; we pass closure_reg as C.
  // class_declaration loads the class into a known register before calling method().
  // We emit METHOD with A = class_reg (passed via reg_top context).
  // Actually, in class_declaration, the class is at a known local.
  // Let's use a simple convention: the class is at the register just before
  // the closure we just allocated.
  // Actually, class_declaration will handle METHOD emission. We just set last_expr_.
  // Wait -- looking at the old compiler, method() directly emits OP_METHOD.
  // The class is on top of stack in old compiler. In register-based, the class
  // register is known by class_declaration. Let's do the emit here with the
  // understanding that class_declaration sets up the class register.

  // We need to know which register holds the class. class_declaration will
  // call named_variable to load it before each method group. So it's in last_expr_
  // of the *caller* context. We can't access that here directly.
  // Instead, class_declaration will handle the METHOD emission after function() returns.
  // Let's NOT emit METHOD here; let class_declaration do it.
  // So we just leave closure in last_expr_.
  last_expr_ = {ExprDesc::VREG, closure_reg};
}

void Compiler::class_declaration() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect class name.");
  Token class_name = ps_->previous;
  u16_t name_constant = identifier_constant(ps_->previous);
  declare_variable();

  // Create class and define variable
  u8_t class_reg;
  if (scope_depth_ > 0) {
    class_reg = static_cast<u8_t>(local_count_ - 1);
    if (reg_top_ <= class_reg) {
      reg_top_ = class_reg + 1;
      check_stack(reg_top_);
    }
  } else {
    class_reg = alloc_reg();
  }
  emit_instr(encode_ABx(OpCode::OP_CLASS, class_reg, name_constant));

  if (scope_depth_ > 0) {
    mark_initialized();
  } else {
    emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, class_reg, name_constant));
  }

  ClassCompiler class_compiler;
  class_compiler.enclosing = ps_->current_class;
  class_compiler.has_superclass = false;
  ps_->current_class = &class_compiler;

  if (match(TokenType::TOKEN_COLON)) {
    consume(TokenType::TOKEN_IDENTIFIER, "Expect superclass name.");
    // Load superclass
    ExprDesc super_expr;
    named_variable(ps_->previous, false);
    super_expr = last_expr_;

    if (class_name.lexeme == ps_->previous.lexeme) {
      error("A class can't inherit from itself.");
    }

    begin_scope();
    add_local(Token{TokenType::TOKEN_SUPER, "super", ps_->previous.line});
    mark_initialized();
    u8_t super_slot = static_cast<u8_t>(local_count_ - 1);
    u8_t super_reg = discharge(super_expr);
    if (super_reg != super_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, super_slot, super_reg, 0));
      if (super_expr.kind == ExprDesc::VREG) free_reg(super_reg);
    }

    // Load class for INHERIT
    u8_t cr = alloc_reg();
    if (scope_depth_ > 1) {
      // class is a local in enclosing scope
      int cs = resolve_local(class_name);
      if (cs != -1) {
        emit_instr(encode_ABC(OpCode::OP_MOVE, cr, static_cast<u8_t>(cs), 0));
      } else {
        u16_t ki = identifier_constant(class_name);
        emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, cr, ki));
      }
    } else {
      u16_t ki = identifier_constant(class_name);
      emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, cr, ki));
    }
    // INHERIT R(A).inherit(R(B)) — A=class, B=super
    emit_instr(encode_ABC(OpCode::OP_INHERIT, cr, super_slot, 0));
    free_reg(cr);

    class_compiler.has_superclass = true;
  }

  // Helper lambda to load class register
  auto load_class_reg = [&]() -> u8_t {
    u8_t cr = alloc_reg();
    int cs = resolve_local(class_name);
    if (cs != -1) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, cr, static_cast<u8_t>(cs), 0));
    } else {
      u16_t ki = identifier_constant(class_name);
      emit_instr(encode_ABx(OpCode::OP_GETGLOBAL, cr, ki));
    }
    return cr;
  };

  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before class body.");
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    if (match(TokenType::TOKEN_ABSTRACT)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect abstract method name.");
      u16_t constant = identifier_constant(ps_->previous);

      if (ps_->previous.lexeme == "init") {
        error("Initializer can't be abstract.");
      }

      function(FunctionType::TYPE_METHOD);
      u8_t closure_reg = discharge(last_expr_);
      u8_t cr = load_class_reg();
      emit_instr(encode_ABC(OpCode::OP_ABSTMETH, cr, static_cast<u8_t>(constant), closure_reg));
      free_reg(cr);
      free_reg(closure_reg);
    } else if (match(TokenType::TOKEN_STATIC)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect static method name.");
      u16_t constant = identifier_constant(ps_->previous);

      if (ps_->previous.lexeme == "init") {
        error("Initializer can't be static.");
      }

      class_compiler.is_static_method = true;
      function(FunctionType::TYPE_FUNCTION);
      class_compiler.is_static_method = false;

      u8_t closure_reg = discharge(last_expr_);
      u8_t cr = load_class_reg();
      emit_instr(encode_ABC(OpCode::OP_STATICMETH, cr, static_cast<u8_t>(constant), closure_reg));
      free_reg(cr);
      free_reg(closure_reg);
    } else if (check(TokenType::TOKEN_IDENTIFIER) &&
               (ps_->current.lexeme == "get" || ps_->current.lexeme == "set")) {
      bool is_getter = ps_->current.lexeme == "get";
      advance();
      consume(TokenType::TOKEN_IDENTIFIER, "Expect property name.");
      u16_t constant = identifier_constant(ps_->previous);

      function(FunctionType::TYPE_METHOD);
      u8_t closure_reg = discharge(last_expr_);
      u8_t cr = load_class_reg();

      if (is_getter) {
        emit_instr(encode_ABC(OpCode::OP_GETTER, cr, static_cast<u8_t>(constant), closure_reg));
      } else {
        emit_instr(encode_ABC(OpCode::OP_SETTER, cr, static_cast<u8_t>(constant), closure_reg));
      }
      free_reg(cr);
      free_reg(closure_reg);
    } else {
      // Regular method
      consume(TokenType::TOKEN_IDENTIFIER, "Expect method name.");
      u16_t constant = identifier_constant(ps_->previous);

      FunctionType ftype = FunctionType::TYPE_METHOD;
      if (ps_->previous.lexeme == "init") {
        ftype = FunctionType::TYPE_INITIALIZER;
      }
      function(ftype);

      u8_t closure_reg = discharge(last_expr_);
      u8_t cr = load_class_reg();
      emit_instr(encode_ABC(OpCode::OP_METHOD, cr, static_cast<u8_t>(constant), closure_reg));
      free_reg(cr);
      free_reg(closure_reg);
    }
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

  if (scope_depth_ == 0) {
    free_reg(class_reg);
  }

  if (class_compiler.has_superclass) {
    end_scope();
  }

  ps_->current_class = ps_->current_class->enclosing;
}

void Compiler::import_declaration() noexcept {
  consume(TokenType::TOKEN_STRING, "Expect module path string.");
  sz_t ki = make_constant(Value(static_cast<Object*>(
      VM::get_instance().copy_string(ps_->previous.lexeme.data() + 1,
                     ps_->previous.lexeme.length() - 2))));

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import path.");

  u8_t path_reg = alloc_reg();
  emit_instr(encode_ABx(OpCode::OP_LOADK, path_reg, static_cast<u16_t>(ki)));
  emit_instr(encode_ABC(OpCode::OP_IMPORT, path_reg, 0, 0));
  free_reg(path_reg);
}

void Compiler::expression_statement() noexcept {
  expression();
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after expression.");
  // Free the register holding the result (no POP needed in register-based)
  u8_t reg = discharge(last_expr_);
  free_reg(reg);
}

void Compiler::print_statement() noexcept {
  expression();
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after value.");
  u8_t reg = discharge(last_expr_);
  emit_instr(encode_ABC(OpCode::OP_PRINT, reg, 0, 0));
  free_reg(reg);
}

void Compiler::if_statement() noexcept {
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  u8_t cond_reg = discharge(last_expr_);

  // TEST R(cond), 0 — if truthy, skip next (the JMP)
  //   If falsy, execute JMP to else
  emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
  sz_t then_jump = emit_jump(OpCode::OP_JMP);
  free_reg(cond_reg);

  statement();

  sz_t else_jump = emit_jump(OpCode::OP_JMP);

  patch_jump(then_jump);

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

  u8_t cond_reg = discharge(last_expr_);

  // If falsy, exit
  emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
  sz_t exit_jump = emit_jump(OpCode::OP_JMP);
  free_reg(cond_reg);

  current_loop_ = &loop;
  statement();
  current_loop_ = loop.enclosing;

  emit_loop(loop_start);

  patch_jump(exit_jump);

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::for_statement() noexcept {
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

  // Check for for-in
  if (match(TokenType::TOKEN_VAR)) {
    consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name.");
    Token var_name = ps_->previous;

    if (match(TokenType::TOKEN_IN)) {
      for_in_statement(var_name);
      return;
    }

    // Standard for with var initializer
    LoopContext loop;
    loop.enclosing = current_loop_;
    loop.break_depth = scope_depth_;

    begin_scope();
    loop.continue_depth = scope_depth_;

    // Declare the variable
    declare_variable();
    u8_t local_slot = static_cast<u8_t>(local_count_ - 1);
    if (match(TokenType::TOKEN_EQUAL)) {
      expression();
      expr_to_reg(last_expr_, local_slot);
    } else {
      emit_instr(encode_ABC(OpCode::OP_LOADNIL, local_slot, 0, 0));
    }
    mark_initialized();
    if (reg_top_ <= local_slot) {
      reg_top_ = local_slot + 1;
      check_stack(reg_top_);
    }
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    sz_t loop_start = current_chunk().count();

    sz_t exit_jump = static_cast<sz_t>(-1);
    if (!match(TokenType::TOKEN_SEMICOLON)) {
      expression();
      u8_t cond_reg = discharge(last_expr_);
      consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after loop condition.");
      emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
      exit_jump = emit_jump(OpCode::OP_JMP);
      free_reg(cond_reg);
    }

    if (!match(TokenType::TOKEN_RIGHT_PAREN)) {
      sz_t body_jump = emit_jump(OpCode::OP_JMP);
      sz_t increment_start = current_chunk().count();
      expression();
      u8_t inc_reg = discharge(last_expr_);
      free_reg(inc_reg);
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
    // No initializer
  } else {
    expression_statement();
  }

  sz_t loop_start = current_chunk().count();

  // Condition
  sz_t exit_jump = static_cast<sz_t>(-1);
  if (!match(TokenType::TOKEN_SEMICOLON)) {
    expression();
    u8_t cond_reg = discharge(last_expr_);
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    emit_instr(encode_ABC(OpCode::OP_TEST, cond_reg, 0, 0));
    exit_jump = emit_jump(OpCode::OP_JMP);
    free_reg(cond_reg);
  }

  // Increment
  if (!match(TokenType::TOKEN_RIGHT_PAREN)) {
    sz_t body_jump = emit_jump(OpCode::OP_JMP);
    sz_t increment_start = current_chunk().count();
    expression();
    u8_t inc_reg = discharge(last_expr_);
    free_reg(inc_reg);
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
  }

  end_scope();

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::for_in_statement(Token var_name) noexcept {
  LoopContext loop;
  loop.enclosing = current_loop_;
  loop.break_depth = scope_depth_;

  begin_scope(); // outer scope for __seq, __idx

  // Compile iterable expression
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after for-in clause.");

  // Create hidden local __seq
  u8_t seq_slot = static_cast<u8_t>(local_count_);
  {
    u8_t seq_reg = discharge(last_expr_);
    if (reg_top_ <= seq_slot) { reg_top_ = seq_slot; alloc_reg(); }
    if (seq_reg != seq_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, seq_slot, seq_reg, 0));
      if (seq_reg >= static_cast<u8_t>(local_count_)) free_reg(seq_reg);
    }
    Token seq_token{TokenType::TOKEN_IDENTIFIER, " __seq", 0};
    add_local(seq_token);
    mark_initialized();
  }

  // Create hidden local __idx = 0
  u8_t idx_slot = static_cast<u8_t>(local_count_);
  {
    sz_t ki = make_constant(Value(static_cast<i64_t>(0)));
    if (reg_top_ <= idx_slot) { reg_top_ = idx_slot; alloc_reg(); }
    emit_instr(encode_ABx(OpCode::OP_LOADK, idx_slot, static_cast<u16_t>(ki)));
    Token idx_token{TokenType::TOKEN_IDENTIFIER, " __idx", 0};
    add_local(idx_token);
    mark_initialized();
  }

  // Loop start
  sz_t loop_start = current_chunk().count();
  loop.continue_target = loop_start;
  loop.continue_depth = scope_depth_;

  // FORITER: seq=R(A), idx=R(A+1), elem→R(A+2); sBx=exit offset
  sz_t foriter_offset = current_chunk().count();
  emit_instr(encode_AsBx(OpCode::OP_FORITER, seq_slot, 0)); // placeholder sBx

  // Inner scope for loop variable
  begin_scope();

  // Element is at R(seq_slot + 2) after FORITER
  // We need to ensure that slot exists
  u8_t elem_slot = static_cast<u8_t>(local_count_);
  if (reg_top_ <= elem_slot) {
    reg_top_ = elem_slot + 1;
    check_stack(reg_top_);
  }
  add_local(var_name);
  mark_initialized();

  // Body
  current_loop_ = &loop;
  statement();
  current_loop_ = loop.enclosing;

  end_scope(); // pops loop variable

  emit_loop(loop_start);

  // Patch FORITER exit
  int exit_offset = static_cast<int>(current_chunk().count()) - static_cast<int>(foriter_offset) - 1;
  current_chunk()[foriter_offset] = encode_AsBx(OpCode::OP_FORITER, seq_slot, exit_offset);

  end_scope(); // pops __idx, __seq

  for (sz_t offset : loop.break_jumps) {
    patch_jump(offset);
  }
}

void Compiler::break_statement() noexcept {
  if (current_loop_ == nullptr) {
    error("Can't use 'break' outside of a loop or switch.");
    return;
  }

  // Close upvalues for locals deeper than the loop's scope
  for (int i = local_count_ - 1; i >= 0 && locals_[i].depth > current_loop_->break_depth; i--) {
    if (locals_[i].is_captured) {
      emit_instr(encode_ABC(OpCode::OP_CLOSE, static_cast<u8_t>(i), 0, 0));
      break; // CLOSE handles all upvalues >= R(i)
    }
  }

  sz_t jump = emit_jump(OpCode::OP_JMP);
  current_loop_->break_jumps.push_back(jump);

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after 'break'.");
}

void Compiler::continue_statement() noexcept {
  if (current_loop_ == nullptr) {
    error("Can't use 'continue' outside of a loop.");
    return;
  }

  // Close upvalues for locals deeper than the loop body's scope
  for (int i = local_count_ - 1; i >= 0 && locals_[i].depth > current_loop_->continue_depth; i--) {
    if (locals_[i].is_captured) {
      emit_instr(encode_ABC(OpCode::OP_CLOSE, static_cast<u8_t>(i), 0, 0));
      break;
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
    u8_t ret_reg = discharge(last_expr_);
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after return value.");
    emit_instr(encode_ABC(OpCode::OP_RETURN, ret_reg, 2, 0));
    free_reg(ret_reg);
  }
}

void Compiler::try_statement() noexcept {
  // try { body } catch (name) { handler }

  // OP_TRY with placeholder sBx to catch block
  sz_t try_offset = emit_jump(OpCode::OP_TRY);

  // Compile try body
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' after 'try'.");
  begin_scope();
  block();
  end_scope();

  // Normal exit: pop handler and jump over catch
  emit_instr(encode_ABC(static_cast<OpCode>(OpCode::OP_ENDTRY), 0, 0, 0));
  sz_t end_jump = emit_jump(OpCode::OP_JMP);

  // Patch try to here
  patch_jump(try_offset);

  // Parse catch
  consume(TokenType::TOKEN_CATCH, "Expect 'catch' after try block.");
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'catch'.");
  consume(TokenType::TOKEN_IDENTIFIER, "Expect variable name.");
  Token error_name = ps_->previous;
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after catch variable.");

  // Catch body: exception is in a register placed by the VM
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before catch body.");
  begin_scope();
  add_local(error_name);
  mark_initialized();
  // Patch the OP_TRY's A field with the catch variable register
  {
    u8_t catch_reg = static_cast<u8_t>(local_count_ - 1);
    Instruction old_instr = current_chunk()[try_offset];
    int sBx = decode_sBx(old_instr);
    current_chunk()[try_offset] = encode_AsBx(OpCode::OP_TRY, catch_reg, sBx);
  }
  block();
  end_scope();

  patch_jump(end_jump);
}

void Compiler::throw_statement() noexcept {
  expression();
  u8_t reg = discharge(last_expr_);
  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after throw value.");
  emit_instr(encode_ABC(OpCode::OP_THROW, reg, 0, 0));
  free_reg(reg);
}

void Compiler::defer_statement() noexcept {
  Compiler compiler(*ps_, FunctionType::TYPE_FUNCTION);
  compiler.begin_scope();

  compiler.statement();

  ObjFunction* wrapper = compiler.end_compiler();
  wrapper->set_name(VM::get_instance().copy_string("<defer>", 7));

  sz_t closure_index = make_constant(Value(static_cast<Object*>(wrapper)));
  u8_t dest = alloc_reg();
  emit_instr(encode_ABx(OpCode::OP_CLOSURE, dest, static_cast<u16_t>(closure_index)));

  for (int i = 0; i < wrapper->upvalue_count(); i++) {
    emit_instr(encode_ABx(OpCode::OP_EXTRAARG,
        compiler.upvalues_[i].is_local ? 1 : 0,
        static_cast<u16_t>(compiler.upvalues_[i].index)));
  }

  emit_instr(encode_ABC(OpCode::OP_DEFER, dest, 0, 0));
  free_reg(dest);
}

void Compiler::switch_statement() noexcept {
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after value.");
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before switch cases.");

  begin_scope();

  // Store switch value as a hidden local
  u8_t switch_slot = static_cast<u8_t>(local_count_);
  {
    u8_t val_reg = discharge(last_expr_);
    if (reg_top_ <= switch_slot) { reg_top_ = switch_slot; alloc_reg(); }
    if (val_reg != switch_slot) {
      emit_instr(encode_ABC(OpCode::OP_MOVE, switch_slot, val_reg, 0));
      if (val_reg >= static_cast<u8_t>(local_count_)) free_reg(val_reg);
    }
    Token switch_token{TokenType::TOKEN_IDENTIFIER, " switch", ps_->previous.line};
    add_local(switch_token);
    mark_initialized();
  }

  LoopContext switch_ctx;
  switch_ctx.enclosing = current_loop_;
  switch_ctx.break_depth = scope_depth_;
  current_loop_ = &switch_ctx;

  std::vector<sz_t> case_ends;
  bool had_default = false;

  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    if (match(TokenType::TOKEN_CASE)) {
      // Compare switch value with case expression
      expression();
      u8_t case_reg = discharge(last_expr_);
      u8_t eq_reg = alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_EQ, eq_reg, switch_slot, case_reg));
      free_reg(case_reg);

      // TEST R(eq), 0 — if truthy (equal), skip JMP
      emit_instr(encode_ABC(OpCode::OP_TEST, eq_reg, 0, 0));
      sz_t next_case = emit_jump(OpCode::OP_JMP);
      free_reg(eq_reg);

      consume(TokenType::TOKEN_COLON, "Expect ':' after case value.");

      while (!check(TokenType::TOKEN_CASE) && !check(TokenType::TOKEN_DEFAULT) &&
             !check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
        declaration();
      }

      case_ends.push_back(emit_jump(OpCode::OP_JMP));
      patch_jump(next_case);
    } else if (match(TokenType::TOKEN_DEFAULT)) {
      if (had_default) {
        error("Already has a default case in switch statement.");
      }
      had_default = true;

      consume(TokenType::TOKEN_COLON, "Expect ':' after 'default'.");

      while (!check(TokenType::TOKEN_CASE) && !check(TokenType::TOKEN_DEFAULT) &&
             !check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
        declaration();
      }
    } else {
      error("Expect 'case' or 'default' in switch statement.");
      advance();
    }
  }

  for (sz_t offset : case_ends) {
    patch_jump(offset);
  }

  current_loop_ = switch_ctx.enclosing;

  for (sz_t offset : switch_ctx.break_jumps) {
    patch_jump(offset);
  }

  end_scope();

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
    sz_t path_ki = make_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(ps_->previous.lexeme.data() + 1,
                       ps_->previous.lexeme.length() - 2))));
    consume(TokenType::TOKEN_IMPORT, "Expect 'import' after module path.");
    consume(TokenType::TOKEN_IDENTIFIER, "Expect name to import.");
    sz_t name_ki = make_constant(Value(static_cast<Object*>(
        VM::get_instance().copy_string(ps_->previous.lexeme.data(), ps_->previous.lexeme.length()))));

    if (match(TokenType::TOKEN_AS)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect alias name after 'as'.");
      sz_t alias_ki = make_constant(Value(static_cast<Object*>(
          VM::get_instance().copy_string(ps_->previous.lexeme.data(), ps_->previous.lexeme.length()))));

      u8_t path_reg = alloc_reg();
      u8_t name_reg = alloc_reg();
      u8_t alias_reg = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_LOADK, path_reg, static_cast<u16_t>(path_ki)));
      emit_instr(encode_ABx(OpCode::OP_LOADK, name_reg, static_cast<u16_t>(name_ki)));
      emit_instr(encode_ABx(OpCode::OP_LOADK, alias_reg, static_cast<u16_t>(alias_ki)));
      emit_instr(encode_ABC(OpCode::OP_IMPALIAS, path_reg, name_reg, alias_reg));
      free_reg(alias_reg);
      free_reg(name_reg);
      free_reg(path_reg);
    } else {
      u8_t path_reg = alloc_reg();
      u8_t name_reg = alloc_reg();
      emit_instr(encode_ABx(OpCode::OP_LOADK, path_reg, static_cast<u16_t>(path_ki)));
      emit_instr(encode_ABx(OpCode::OP_LOADK, name_reg, static_cast<u16_t>(name_ki)));
      emit_instr(encode_ABC(OpCode::OP_IMPFROM, path_reg, name_reg, 0));
      free_reg(name_reg);
      free_reg(path_reg);
    }

    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import statement.");
  } else {
    statement();
  }

  if (ps_->panic_mode) synchronize();
}

// =============================================================================
// End compiler
// =============================================================================

ObjFunction* Compiler::end_compiler() noexcept {
  emit_return();
  ObjFunction* function = function_;

  function->set_max_stack_size(max_reg_);

#ifdef MAPLE_DEBUG_PRINT
  if (!ps_->had_error) {
    disassemble_chunk(current_chunk(),
        function->name() != nullptr ? strv_t(function->name()->value()) : "<script>");
  }
#endif

  ps_->current_compiler = enclosing_;
  return function;
}

// =============================================================================
// Public API
// =============================================================================

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

ObjFunction* compile(strv_t source, strv_t script_path,
                     std::vector<Diagnostic>& diagnostics) noexcept {
  ParseState ps;
  ps.scanner.init(source);
  ps.script_path = str_t(script_path);
  ps.diagnostics = &diagnostics;
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
