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
#include <cstdlib>
#include <iostream>
#include <format>
#include "Compiler.hh"
#include "Memory.hh"
#include "Debug.hh"

namespace ms {

// Forward declarations from VM
ObjString* vm_copy_string(cstr_t chars, sz_t length) noexcept;

enum class Precedence : int {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY,
};

class Compiler;

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

struct ClassCompiler {
  ClassCompiler* enclosing{nullptr};
  bool has_superclass{false};
};

class Compiler {
  // Parser state (shared across all Compiler instances via static)
  static Scanner scanner_;
  static Token current_;
  static Token previous_;
  static bool had_error_;
  static bool panic_mode_;
  static ClassCompiler* current_class_;

  // Compiler state (per function scope)
  Compiler* enclosing_{nullptr};
  ObjFunction* function_{nullptr};
  FunctionType type_;

  Local locals_[kUINT8_COUNT];
  int local_count_{0};
  Upvalue upvalues_[kUINT8_COUNT];
  int scope_depth_{0};

  static Compiler* current_compiler_;

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
  void emit_return() noexcept;
  void emit_constant(Value value) noexcept;
  sz_t emit_jump(OpCode op) noexcept;
  void patch_jump(sz_t offset) noexcept;
  void emit_loop(sz_t loop_start) noexcept;
  u8_t make_constant(Value value) noexcept;

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
  void return_statement() noexcept;
  void function(FunctionType type) noexcept;
  void method() noexcept;
  void synchronize() noexcept;

  ObjFunction* end_compiler() noexcept;

public:
  Compiler(FunctionType type) noexcept;

  // Expression parsers — public so ParseRule table can take member-function pointers
  void grouping(bool can_assign) noexcept;
  void number(bool can_assign) noexcept;
  void string(bool can_assign) noexcept;
  void variable(bool can_assign) noexcept;
  void unary(bool can_assign) noexcept;
  void binary(bool can_assign) noexcept;
  void literal(bool can_assign) noexcept;
  void and_(bool can_assign) noexcept;
  void or_(bool can_assign) noexcept;
  void call(bool can_assign) noexcept;
  void dot(bool can_assign) noexcept;
  void this_(bool can_assign) noexcept;
  void super_(bool can_assign) noexcept;
  u8_t argument_list() noexcept;

  friend ObjFunction* compile(strv_t source) noexcept;
  friend void mark_compiler_roots() noexcept;
};

// Static member definitions
Scanner Compiler::scanner_;
Token Compiler::current_;
Token Compiler::previous_;
bool Compiler::had_error_ = false;
bool Compiler::panic_mode_ = false;
ClassCompiler* Compiler::current_class_ = nullptr;
Compiler* Compiler::current_compiler_ = nullptr;

// Parse rule table
static ParseRule rules[] = {
  // TOKEN_LEFT_PAREN
  { &Compiler::grouping, &Compiler::call,    Precedence::PREC_CALL },
  // TOKEN_RIGHT_PAREN
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_LEFT_BRACE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_RIGHT_BRACE
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
  // TOKEN_COLON
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_IDENTIFIER
  { &Compiler::variable, nullptr,            Precedence::PREC_NONE },
  // TOKEN_STRING
  { &Compiler::string,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_NUMBER
  { &Compiler::number,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_AND
  { nullptr,             &Compiler::and_,    Precedence::PREC_AND },
  // TOKEN_CLASS
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_ELSE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_FALSE
  { &Compiler::literal,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_FOR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_FUN
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_IF
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
  // TOKEN_SUPER
  { &Compiler::super_,   nullptr,            Precedence::PREC_NONE },
  // TOKEN_THIS
  { &Compiler::this_,    nullptr,            Precedence::PREC_NONE },
  // TOKEN_TRUE
  { &Compiler::literal,  nullptr,            Precedence::PREC_NONE },
  // TOKEN_VAR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_WHILE
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_ERROR
  { nullptr,             nullptr,            Precedence::PREC_NONE },
  // TOKEN_EOF
  { nullptr,             nullptr,            Precedence::PREC_NONE },
};

// --- Compiler implementation ---

Compiler::Compiler(FunctionType type) noexcept
    : type_(type) {
  enclosing_ = current_compiler_;
  current_compiler_ = this;

  function_ = allocate_object<ObjFunction>();

  if (type != FunctionType::TYPE_SCRIPT) {
    function_->name_ = vm_copy_string(
        previous_.lexeme.data(), previous_.lexeme.length());
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

void Compiler::error_at(const Token& token, strv_t message) noexcept {
  if (panic_mode_) return;
  panic_mode_ = true;

  std::cerr << std::format("[line {}] Error", token.line);

  if (token.type == TokenType::TOKEN_EOF) {
    std::cerr << " at end";
  } else if (token.type != TokenType::TOKEN_ERROR) {
    std::cerr << std::format(" at '{}'", token.lexeme);
  }

  std::cerr << ": " << message << std::endl;
  had_error_ = true;
}

void Compiler::error(strv_t message) noexcept {
  error_at(previous_, message);
}

void Compiler::error_at_current(strv_t message) noexcept {
  error_at(current_, message);
}

void Compiler::advance() noexcept {
  previous_ = current_;

  for (;;) {
    current_ = scanner_.scan_token();
    if (current_.type != TokenType::TOKEN_ERROR) break;

    error_at_current(current_.lexeme);
  }
}

void Compiler::consume(TokenType type, strv_t message) noexcept {
  if (current_.type == type) {
    advance();
    return;
  }
  error_at_current(message);
}

bool Compiler::check(TokenType type) const noexcept {
  return current_.type == type;
}

bool Compiler::match(TokenType type) noexcept {
  if (!check(type)) return false;
  advance();
  return true;
}

Chunk& Compiler::current_chunk() noexcept {
  return function_->chunk_;
}

void Compiler::emit_byte(u8_t byte) noexcept {
  current_chunk().write(byte, previous_.line);
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

void Compiler::emit_return() noexcept {
  if (type_ == FunctionType::TYPE_INITIALIZER) {
    emit_op_byte(OpCode::OP_GET_LOCAL, 0);
  } else {
    emit_op(OpCode::OP_NIL);
  }
  emit_op(OpCode::OP_RETURN);
}

u8_t Compiler::make_constant(Value value) noexcept {
  sz_t constant = current_chunk().add_constant(value);
  if (constant > 255) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<u8_t>(constant);
}

void Compiler::emit_constant(Value value) noexcept {
  emit_op_byte(OpCode::OP_CONSTANT, make_constant(value));
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
  return make_constant(Value(static_cast<Object*>(
      vm_copy_string(name.lexeme.data(), name.lexeme.length()))));
}

u8_t Compiler::parse_variable(strv_t error_msg) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, error_msg);

  declare_variable();
  if (scope_depth_ > 0) return 0;

  return identifier_constant(previous_);
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

  Token& name = previous_;

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
  int upvalue_count = function_->upvalue_count_;

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
  return function_->upvalue_count_++;
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
  ParseFn prefix_rule = get_rule(previous_.type).prefix;
  if (prefix_rule == nullptr) {
    error("Expect expression.");
    return;
  }

  bool can_assign = (precedence <= Precedence::PREC_ASSIGNMENT);
  (this->*prefix_rule)(can_assign);

  while (precedence <= get_rule(current_.type).precedence) {
    advance();
    ParseFn infix_rule = get_rule(previous_.type).infix;
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
  double value = std::strtod(str_t(previous_.lexeme).c_str(), nullptr);
  emit_constant(Value(value));
}

void Compiler::string(bool /*can_assign*/) noexcept {
  // Trim the surrounding quotes
  emit_constant(Value(static_cast<Object*>(
      vm_copy_string(previous_.lexeme.data() + 1,
                     previous_.lexeme.length() - 2))));
}

void Compiler::variable(bool can_assign) noexcept {
  named_variable(previous_, can_assign);
}

void Compiler::unary(bool /*can_assign*/) noexcept {
  TokenType operator_type = previous_.type;

  parse_precedence(Precedence::PREC_UNARY);

  switch (operator_type) {
  case TokenType::TOKEN_BANG:  emit_op(OpCode::OP_NOT); break;
  case TokenType::TOKEN_MINUS: emit_op(OpCode::OP_NEGATE); break;
  default: return;
  }
}

void Compiler::binary(bool /*can_assign*/) noexcept {
  TokenType operator_type = previous_.type;
  const ParseRule& rule = get_rule(operator_type);
  parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

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
  default: return;
  }
}

void Compiler::literal(bool /*can_assign*/) noexcept {
  switch (previous_.type) {
  case TokenType::TOKEN_FALSE: emit_op(OpCode::OP_FALSE); break;
  case TokenType::TOKEN_NIL:   emit_op(OpCode::OP_NIL); break;
  case TokenType::TOKEN_TRUE:  emit_op(OpCode::OP_TRUE); break;
  default: return;
  }
}

void Compiler::and_(bool /*can_assign*/) noexcept {
  sz_t end_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);

  emit_op(OpCode::OP_POP);
  parse_precedence(Precedence::PREC_AND);

  patch_jump(end_jump);
}

void Compiler::or_(bool /*can_assign*/) noexcept {
  sz_t else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  sz_t end_jump = emit_jump(OpCode::OP_JUMP);

  patch_jump(else_jump);
  emit_op(OpCode::OP_POP);

  parse_precedence(Precedence::PREC_OR);
  patch_jump(end_jump);
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
}

void Compiler::dot(bool can_assign) noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect property name after '.'.");
  u8_t name = identifier_constant(previous_);

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    emit_op_byte(OpCode::OP_SET_PROPERTY, name);
  } else if (match(TokenType::TOKEN_LEFT_PAREN)) {
    u8_t arg_count = argument_list();
    emit_bytes(static_cast<u8_t>(OpCode::OP_INVOKE), name);
    emit_byte(arg_count);
  } else {
    emit_op_byte(OpCode::OP_GET_PROPERTY, name);
  }
}

void Compiler::this_(bool /*can_assign*/) noexcept {
  if (current_class_ == nullptr) {
    error("Can't use 'this' outside of a class.");
    return;
  }
  variable(false);
}

void Compiler::super_(bool /*can_assign*/) noexcept {
  if (current_class_ == nullptr) {
    error("Can't use 'super' outside of a class.");
  } else if (!current_class_->has_superclass) {
    error("Can't use 'super' in a class with no superclass.");
  }

  consume(TokenType::TOKEN_DOT, "Expect '.' after 'super'.");
  consume(TokenType::TOKEN_IDENTIFIER, "Expect superclass method name.");
  u8_t name = identifier_constant(previous_);

  named_variable(Token{TokenType::TOKEN_THIS, "this", previous_.line}, false);

  if (match(TokenType::TOKEN_LEFT_PAREN)) {
    u8_t arg_count = argument_list();
    named_variable(Token{TokenType::TOKEN_SUPER, "super", previous_.line}, false);
    emit_bytes(static_cast<u8_t>(OpCode::OP_SUPER_INVOKE), name);
    emit_byte(arg_count);
  } else {
    named_variable(Token{TokenType::TOKEN_SUPER, "super", previous_.line}, false);
    emit_op_byte(OpCode::OP_GET_SUPER, name);
  }
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
  Compiler compiler(type);
  compiler.begin_scope();

  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after function name.");
  if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
    do {
      compiler.function_->arity_++;
      if (compiler.function_->arity_ > 255) {
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
  emit_op_byte(OpCode::OP_CLOSURE, make_constant(Value(static_cast<Object*>(function))));

  for (int i = 0; i < function->upvalue_count_; i++) {
    emit_byte(compiler.upvalues_[i].is_local ? 1 : 0);
    emit_byte(compiler.upvalues_[i].index);
  }
}

void Compiler::fun_declaration() noexcept {
  u8_t global = parse_variable("Expect function name.");
  mark_initialized();
  function(FunctionType::TYPE_FUNCTION);
  define_variable(global);
}

void Compiler::method() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect method name.");
  u8_t constant = identifier_constant(previous_);

  FunctionType type = FunctionType::TYPE_METHOD;
  if (previous_.lexeme == "init") {
    type = FunctionType::TYPE_INITIALIZER;
  }
  function(type);

  emit_op_byte(OpCode::OP_METHOD, constant);
}

void Compiler::class_declaration() noexcept {
  consume(TokenType::TOKEN_IDENTIFIER, "Expect class name.");
  Token class_name = previous_;
  u8_t name_constant = identifier_constant(previous_);
  declare_variable();

  emit_op_byte(OpCode::OP_CLASS, name_constant);
  define_variable(name_constant);

  ClassCompiler class_compiler;
  class_compiler.enclosing = current_class_;
  class_compiler.has_superclass = false;
  current_class_ = &class_compiler;

  if (match(TokenType::TOKEN_COLON)) {
    consume(TokenType::TOKEN_IDENTIFIER, "Expect superclass name.");
    variable(false);

    if (class_name.lexeme == previous_.lexeme) {
      error("A class can't inherit from itself.");
    }

    begin_scope();
    add_local(Token{TokenType::TOKEN_SUPER, "super", previous_.line});
    define_variable(0);

    named_variable(class_name, false);
    emit_op(OpCode::OP_INHERIT);
    class_compiler.has_superclass = true;
  }

  named_variable(class_name, false);
  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' before class body.");
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    method();
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
  emit_op(OpCode::OP_POP);

  if (class_compiler.has_superclass) {
    end_scope();
  }

  current_class_ = current_class_->enclosing;
}

void Compiler::import_declaration() noexcept {
  consume(TokenType::TOKEN_STRING, "Expect module path string.");
  // Trim quotes from the string
  u8_t path_constant = make_constant(Value(static_cast<Object*>(
      vm_copy_string(previous_.lexeme.data() + 1,
                     previous_.lexeme.length() - 2))));

  consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import path.");

  emit_op_byte(OpCode::OP_CONSTANT, path_constant);
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
  sz_t loop_start = current_chunk().count();
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  sz_t exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
  emit_op(OpCode::OP_POP);
  statement();
  emit_loop(loop_start);

  patch_jump(exit_jump);
  emit_op(OpCode::OP_POP);
}

void Compiler::for_statement() noexcept {
  begin_scope();
  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

  // Initializer
  if (match(TokenType::TOKEN_SEMICOLON)) {
    // No initializer.
  } else if (match(TokenType::TOKEN_VAR)) {
    var_declaration();
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

  statement();
  emit_loop(loop_start);

  if (exit_jump != static_cast<sz_t>(-1)) {
    patch_jump(exit_jump);
    emit_op(OpCode::OP_POP);
  }

  end_scope();
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

void Compiler::synchronize() noexcept {
  panic_mode_ = false;

  while (current_.type != TokenType::TOKEN_EOF) {
    if (previous_.type == TokenType::TOKEN_SEMICOLON) return;
    switch (current_.type) {
    case TokenType::TOKEN_CLASS:
    case TokenType::TOKEN_FUN:
    case TokenType::TOKEN_VAR:
    case TokenType::TOKEN_FOR:
    case TokenType::TOKEN_IF:
    case TokenType::TOKEN_WHILE:
    case TokenType::TOKEN_PRINT:
    case TokenType::TOKEN_RETURN:
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
    u8_t path_constant = make_constant(Value(static_cast<Object*>(
        vm_copy_string(previous_.lexeme.data() + 1,
                       previous_.lexeme.length() - 2))));
    consume(TokenType::TOKEN_IMPORT, "Expect 'import' after module path.");
    consume(TokenType::TOKEN_IDENTIFIER, "Expect name to import.");
    u8_t name_constant = identifier_constant(previous_);

    if (match(TokenType::TOKEN_AS)) {
      consume(TokenType::TOKEN_IDENTIFIER, "Expect alias name after 'as'.");
      u8_t alias_constant = identifier_constant(previous_);

      emit_op_byte(OpCode::OP_CONSTANT, path_constant);
      emit_op_byte(OpCode::OP_CONSTANT, name_constant);
      emit_op_byte(OpCode::OP_CONSTANT, alias_constant);
      emit_op(OpCode::OP_IMPORT_ALIAS);
    } else {
      emit_op_byte(OpCode::OP_CONSTANT, path_constant);
      emit_op_byte(OpCode::OP_CONSTANT, name_constant);
      emit_op(OpCode::OP_IMPORT_FROM);
    }

    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after import statement.");
  } else {
    statement();
  }

  if (panic_mode_) synchronize();
}

ObjFunction* Compiler::end_compiler() noexcept {
  emit_return();
  ObjFunction* function = function_;

#ifdef MAPLE_DEBUG_PRINT
  if (!had_error_) {
    disassemble_chunk(current_chunk(),
        function->name_ != nullptr ? strv_t(function->name_->value_) : "<script>");
  }
#endif

  current_compiler_ = enclosing_;
  return function;
}

ObjFunction* compile(strv_t source) noexcept {
  Compiler::scanner_.init(source);
  Compiler::had_error_ = false;
  Compiler::panic_mode_ = false;

  Compiler compiler(FunctionType::TYPE_SCRIPT);
  compiler.advance();

  while (!compiler.match(TokenType::TOKEN_EOF)) {
    compiler.declaration();
  }

  ObjFunction* function = compiler.end_compiler();
  return Compiler::had_error_ ? nullptr : function;
}

void mark_compiler_roots() noexcept {
  Compiler* compiler = Compiler::current_compiler_;
  while (compiler != nullptr) {
    mark_object(compiler->function_);
    compiler = compiler->enclosing_;
  }
}

} // namespace ms
