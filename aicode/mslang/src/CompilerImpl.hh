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
#pragma once

#include <array>
#include <charconv>
#include <cmath>
#include <unordered_map>
#include "Compiler.hh"
#include "VM.hh"
#include "Memory.hh"

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
// Compound assignment helpers — shared by Compiler.cc and CompilerExpr.cc
// =============================================================================

// Returns the arithmetic opcode for a compound assignment token type.
inline OpCode compound_op(TokenType type) noexcept {
  switch (type) {
  case TokenType::TOKEN_PLUS_EQUAL:    return OpCode::OP_ADD;
  case TokenType::TOKEN_MINUS_EQUAL:   return OpCode::OP_SUB;
  case TokenType::TOKEN_STAR_EQUAL:    return OpCode::OP_MUL;
  case TokenType::TOKEN_SLASH_EQUAL:   return OpCode::OP_DIV;
  case TokenType::TOKEN_PERCENT_EQUAL: return OpCode::OP_MOD;
  default: return OpCode::OP_RETURN; // sentinel: not a compound op
  }
}

inline bool is_compound_op(TokenType type) noexcept {
  return compound_op(type) != OpCode::OP_RETURN;
}

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

  // String constant deduplication cache: interned ObjString* → constant pool index
  std::unordered_map<ObjString*, int> str_const_cache_;

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
  void consume_semi(strv_t message) noexcept;
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
  void enum_declaration() noexcept;
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
  void yield_statement() noexcept;
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

inline ParseState* active_parse_state_ = nullptr;

} // namespace ms
