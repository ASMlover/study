// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include <functional>
#include <iostream>
#include <vector>
#include "common.hh"
#include "vm.hh"
#include "token.hh"
#include "scanner.hh"
#include "chunk.hh"
#include "object.hh"
#include "compiler.hh"

namespace clox {

enum class Precedence {
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

template <typename N> inline Precedence operator+(Precedence a, N b) noexcept {
  return as_type<Precedence>(as_type<int>(a) + as_type<int>(b));
}

struct Local {
  Token name;
  int depth;

  Local(const Token& arg_name, int arg_depth = 0) noexcept : name{arg_name}, depth{arg_depth} {}
};

enum class FunctionType {
  TYPE_FUNCTION,
  TYPE_SCRIPT,
};

struct Compiler {
  using ErrorFn = std::function<void (cstr_t)>;

  ObjFunction* function{};
  FunctionType type{FunctionType::TYPE_SCRIPT};
  std::vector<Local> locals;
  int scope_depth{};

  static constexpr sz_t kMaxLocalCount = UINT8_MAX;

  inline bool is_local_count_max() noexcept { return locals.size() >= kMaxLocalCount; }

  inline void set_compiler(int arg_scope_depth = 0,
      FunctionType arg_type = FunctionType::TYPE_SCRIPT) noexcept {
    type = arg_type;
    locals.clear();
    scope_depth = arg_scope_depth;
    function = ObjFunction::create();
  }

  inline void append(const Token& name, int depth) noexcept {
    locals.push_back({name, depth});
  }

  inline int resolve_local(const Token& name, const ErrorFn& error) const noexcept {
    for (int i = as_type<int>(locals.size()) - 1; i >= 0; --i) {
      const Local& local = locals[i];
      if (local.name == name) {
        if (local.depth == -1)
          error("Cannot read local variable in its own initializer");

        return i;
      }
    }
    return -1;
  }
};

class Parser;

struct ParseRule {
  using ParseFn = std::function<void (Parser& parser, bool can_assign)>;

  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

class Parser final : private UnCopyable {
  VM& vm_;
  Scanenr& scanner_;
  Token previous_;
  Token current_;
  bool had_error_{};
  bool panic_mode_{};

  Compiler* current_compiler_{};

  void advance() {
    previous_ = current_;

    for (;;) {
      current_ = scanner_.scan_token();
      if (current_.type() != TokenType::TOKEN_ERROR)
        break;

      error_at_current(current_.as_string());
    }
  }

  void consume(TokenType type, const str_t& message) {
    if (current_.type() == type) {
      advance();
      return;
    }

    error_at_current(message);
  }

  inline bool check(TokenType type) const noexcept { return current_.type() == type; }

  bool match(TokenType type) noexcept {
    if (!check(type))
      return false;

    advance();
    return true;
  }

  inline Chunk* current_chunk() noexcept {
    return current_compiler_->function->chunk();
  }

  inline void error_at_current(const str_t& message) noexcept { error_at(current_, message); }
  inline void error(const str_t& message) noexcept { error_at(previous_, message); }

  void error_at(const Token& token, const str_t& message) noexcept {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "[line " << token.lineno() << "] Error";
    if (token.type() == TokenType::TOKEN_EOF) {
      std::cerr << " at end";
    }
    else if (token.type() == TokenType::TOKEN_ERROR) {
    }
    else {
      std::cerr << " at `" << token.as_string() << "`";
    }
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
  }

  inline void emit_return() noexcept { emit_byte(OpCode::OP_RETURN); }

  template <typename T> inline void emit_byte(T byte) noexcept { current_chunk()->write(byte, previous_.lineno()); }
  template <typename T, typename U> inline void emit_bytes(T byte1, U byte2) noexcept {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  inline void emit_loop(int loop_start) noexcept {
    emit_byte(OpCode::OP_LOOP);

    int offset = as_type<int>(current_chunk()->codes_count()) - loop_start + 2;
    if (offset > UINT16_MAX)
      error("loop body too large");

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
  }

  template <typename T> inline int emit_jump(T instruction) noexcept {
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    return as_type<int>(current_chunk()->codes_count() - 2);
  }

  inline void emit_constant(const Value& value) noexcept {
    emit_bytes(OpCode::OP_CONSTANT, current_chunk()->add_constant(value));
  }

  inline void patch_jump(int offset) noexcept {
    // // -2 to adjust for the bytecode for the jump offset itself
    int jump = as_type<int>(current_chunk()->codes_count() - offset - 2);

    if (jump > UINT16_MAX)
      error("too much code to jump over");

    current_chunk()->set_code(offset, (jump >> 8) & 0xff);
    current_chunk()->set_code(offset + 1, jump & 0xff);
  }

  inline void init_compiler(Compiler* compiler, FunctionType type) noexcept {
    compiler->set_compiler(0, type);
    current_compiler_ = compiler;
  }

  inline ObjFunction* end_compiler() noexcept {
    emit_return();
    ObjFunction* function = current_compiler_->function;

#if defined(_CLOX_DEBUG_PRINT_CODE)
    if (!had_error_) {
      current_chunk()->dis(function->name_ascstr());
    }
#endif

    return function;
  }

  inline void begin_scope() noexcept { current_compiler_->scope_depth++; }
  inline void end_scope() noexcept {
    current_compiler_->scope_depth--;

    while (current_compiler_->locals.size() > 0 &&
        current_compiler_->locals.back().depth > current_compiler_->scope_depth) {
      emit_byte(OpCode::OP_POP);
      current_compiler_->locals.pop_back();
    }
  }

  inline void binary(bool can_assign) noexcept {
    TokenType operator_type = previous_.type();

    const ParseRule& rule = get_rule(operator_type);
    parse_precedence(rule.precedence + 1);

    switch (operator_type) {
    case TokenType::TOKEN_BANG_EQUAL: emit_bytes(OpCode::OP_EQUAL, OpCode::OP_NOT); break;
    case TokenType::TOKEN_EQUAL_EQUAL: emit_byte(OpCode::OP_EQUAL); break;
    case TokenType::TOKEN_GREATER: emit_byte(OpCode::OP_GREATER); break;
    case TokenType::TOKEN_GREATER_EQUAL: emit_bytes(OpCode::OP_GREATER, OpCode::OP_NOT); break;
    case TokenType::TOKEN_LESS: emit_byte(OpCode::OP_LESS); break;
    case TokenType::TOKEN_LESS_EQUAL: emit_bytes(OpCode::OP_LESS, OpCode::OP_NOT); break;
    case TokenType::TOKEN_PLUS: emit_byte(OpCode::OP_ADD); break;
    case TokenType::TOKEN_MINUS: emit_byte(OpCode::OP_SUBTRACT); break;
    case TokenType::TOKEN_STAR: emit_byte(OpCode::OP_MULTIPLY); break;
    case TokenType::TOKEN_SLASH: emit_byte(OpCode::OP_DIVIDE); break;
    default: return; // unreachable
    }
  }

  inline void literal(bool can_assign) noexcept {
    switch (previous_.type()) {
    case TokenType::KEYWORD_FALSE: emit_byte(OpCode::OP_FALSE); break;
    case TokenType::KEYWORD_NIL: emit_byte(OpCode::OP_NIL); break;
    case TokenType::KEYWORD_TRUE: emit_byte(OpCode::OP_TRUE); break;
    default: return; // unreachable
    }
  }

  inline void grouping(bool can_assign) noexcept {
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after expression");
  }

  inline void number(bool can_assign) noexcept {
    double value = previous_.as_numeric();
    emit_constant(value);
  }

  inline void string(bool can_assign) noexcept {
    emit_constant(ObjString::create(previous_.as_string()));
  }

  inline void named_variable(const Token& name, bool can_assign) noexcept {
    auto errfn = [this](cstr_t msg) noexcept { error(msg); };

    OpCode get_op, set_op;
    int arg = current_compiler_->resolve_local(name, errfn);
    if (arg != -1) {
      get_op = OpCode::OP_GET_LOCAL;
      set_op = OpCode::OP_SET_LOCAL;
    }
    else {
      arg = identifier_constant(name);
      get_op = OpCode::OP_GET_GLOBAL;
      set_op = OpCode::OP_SET_GLOBAL;
    }

    if (can_assign && match(TokenType::TOKEN_EQUAL)) {
      expression();
      emit_bytes(set_op, arg);
    }
    else {
      emit_bytes(get_op, arg);
    }
  }

  inline void variable(bool can_assign) noexcept {
    named_variable(previous_, can_assign);
  }

  inline void unary(bool can_assign) noexcept {
    TokenType operator_type = previous_.type();

    // compile the operand
    parse_precedence(Precedence::PREC_UNARY);

    // emit the operator instruction
    switch (operator_type) {
    case TokenType::TOKEN_BANG: emit_byte(OpCode::OP_NOT); break;
    case TokenType::TOKEN_MINUS: emit_byte(OpCode::OP_NEGATE); break;
    default: return; // unreachable
    }
  }

  inline void and_(bool can_assign) noexcept {
    //      left operand expression
    // .--- OP_JUMP_IF_FALSE
    // |    OP_POP
    // |
    // |    right operand expression
    // `--> continues ...

    int end_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);

    emit_byte(OpCode::OP_POP);
    parse_precedence(Precedence::PREC_AND);

    patch_jump(end_jump);
  }

  inline void or_(bool can_assign) noexcept {
    //        left operand expression
    //    .-- OP_JUMP_IF_FALSE
    // .--|-- OP_JUMP
    // |  `-> OP_POP
    // |
    // |      right operand expression
    // `----> continues ...

    int else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OpCode::OP_JUMP);

    patch_jump(else_jump);
    emit_byte(OpCode::OP_POP);

    parse_precedence(Precedence::PREC_OR);
    patch_jump(end_jump);
  }

  const ParseRule& get_rule(TokenType type) const noexcept {
#define _RULE(fn) [](Parser& p, bool b) { p.fn(b); }
    static const ParseRule _rules[] = {
      {_RULE(grouping), nullptr, Precedence::PREC_NONE},      // PUNCTUATOR(LEFT_PAREN, "(")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(RIGHT_PAREN, ")")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(LEFT_BRACE, "{")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(RIGHT_BRACE, "}")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(COMMA, ",")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(DOT, ".")
      {_RULE(unary), _RULE(binary), Precedence::PREC_TERM},   // PUNCTUATOR(MINUS, "-")
      {_RULE(unary), _RULE(binary), Precedence::PREC_TERM},   // PUNCTUATOR(PLUS, "+")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(SEMICOLON, ";")
      {nullptr, _RULE(binary), Precedence::PREC_FACTOR},      // PUNCTUATOR(SLASH, "/")
      {nullptr, _RULE(binary), Precedence::PREC_FACTOR},      // PUNCTUATOR(STAR, "*")

      {_RULE(unary), nullptr, Precedence::PREC_NONE},         // PUNCTUATOR(BANG, "!")
      {nullptr, _RULE(binary), Precedence::PREC_EQUALITY},    // PUNCTUATOR(BANG_EQUAL, "!=")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(EQUAL, "=")
      {nullptr, _RULE(binary), Precedence::PREC_EQUALITY},    // PUNCTUATOR(EQUAL_EQUAL, "==")
      {nullptr, _RULE(binary), Precedence::PREC_COMPARISON},  // PUNCTUATOR(GREATER, ">")
      {nullptr, _RULE(binary), Precedence::PREC_COMPARISON},  // PUNCTUATOR(GREATER_EQUAL, ">=")
      {nullptr, _RULE(binary), Precedence::PREC_COMPARISON},  // PUNCTUATOR(LESS, "<")
      {nullptr, _RULE(binary), Precedence::PREC_COMPARISON},  // PUNCTUATOR(LESS_EQUAL, "<=")

      {_RULE(variable), nullptr, Precedence::PREC_NONE},      // TOKEN(IDENTIFIER, "Token-Identifier")
      {_RULE(string), nullptr, Precedence::PREC_NONE},        // TOKEN(STRING, "Token-String")
      {_RULE(number), nullptr, Precedence::PREC_NONE},        // TOKEN(NUMBER, "Token-Number")

      {nullptr, _RULE(and_), Precedence::PREC_AND},           // KEYWORD(AND, "and")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(CLASS, "class")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(ELSE, "else")
      {_RULE(literal), nullptr, Precedence::PREC_NONE},       // KEYWORD(FALSE, "false")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(FOR, "for")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(FUN, "fun")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(IF, "if")
      {_RULE(literal), nullptr, Precedence::PREC_NONE},       // KEYWORD(NIL, "nil")
      {nullptr, _RULE(or_), Precedence::PREC_OR},             // KEYWORD(OR, "or")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(PRINT, "print")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(RETURN, "return")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(SUPER, "super")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(THIS, "this")
      {_RULE(literal), nullptr, Precedence::PREC_NONE},       // KEYWORD(TRUE, "true")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(VAR, "var")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(WHILE, "while")

      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(ERROR, "Token-Error")
      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(EOF, "Token-Eof")
    };
#undef _RULE

    return _rules[as_type<int>(type)];
  }

  void parse_precedence(Precedence precedence) noexcept {
    advance();
    auto prefix_rule = get_rule(previous_.type()).prefix;
    if (!prefix_rule) {
      error("expect expression");
      return;
    }

    bool can_assign = precedence <= Precedence::PREC_ASSIGNMENT;
    prefix_rule(*this, can_assign);

    while (precedence <= get_rule(current_.type()).precedence) {
      advance();
      auto infix_rule = get_rule(previous_.type()).infix;

      if (infix_rule)
        infix_rule(*this, can_assign);
    }

    if (can_assign && match(TokenType::TOKEN_EQUAL)) {
      error("invalid assignment target");
    }
  }

  inline u8_t parse_variable(const str_t& error_message) noexcept {
    consume(TokenType::TOKEN_IDENTIFIER, error_message);

    declare_variable();
    if (current_compiler_->scope_depth > 0)
      return 0;
    return identifier_constant(previous_);
  }

  inline void mark_initialized() noexcept {
    if (current_compiler_->scope_depth == 0)
      return;
    current_compiler_->locals.back().depth = current_compiler_->scope_depth;
  }

  inline void define_variable(u8_t global) noexcept {
    if (current_compiler_->scope_depth > 0) {
      mark_initialized();
      return;
    }
    emit_bytes(OpCode::OP_DEFINE_GLOBAL, global);
  }

  inline u8_t identifier_constant(const Token& name) noexcept {
    return current_chunk()->add_constant(ObjString::create(name.as_string()));
  }

  inline void add_local(const Token& name) noexcept {
    if (current_compiler_->is_local_count_max()) {
      error("too many local variables in function");
      return;
    }
    current_compiler_->append(name, -1);
  }

  inline void declare_variable() noexcept {
    if (current_compiler_->scope_depth == 0)
      return;

    const Token& name = previous_;
    for (auto it = current_compiler_->locals.rbegin(); it != current_compiler_->locals.rend(); ++it) {
      const Local& local = *it;
      if (local.depth != -1 && local.depth < current_compiler_->scope_depth)
        break;

      if (local.name == name)
        error("already a variable with this name is redefined in this scope");
    }

    add_local(previous_);
  }

  void declaration() noexcept {
    if (match(TokenType::KEYWORD_FUN)) {
      fun_declaration();
    }
    else if (match(TokenType::KEYWORD_VAR)) {
      var_declaration();
    }
    else {
      statement();
    }

    if (panic_mode_)
      synchronize();
  }

  void statement() noexcept {
    if (match(TokenType::KEYWORD_PRINT)) {
      print_statement();
    }
    else if (match(TokenType::KEYWORD_FOR)) {
      for_statement();
    }
    else if (match(TokenType::KEYWORD_IF)) {
      if_statement();
    }
    else if (match(TokenType::KEYWORD_WHILE)) {
      while_statement();
    }
    else if (match(TokenType::TOKEN_LEFT_BRACE)) {
      begin_scope();
      block();
      end_scope();
    }
    else {
      expression_statement();
    }
  }

  void expression() noexcept {
    parse_precedence(Precedence::PREC_ASSIGNMENT);
  }

  void block() noexcept {
    while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
      declaration();
    }
    consume(TokenType::TOKEN_RIGHT_BRACE, "expect `}` after block");
  }

  void function(FunctionType type) noexcept {
    Compiler compiler;
    init_compiler(&compiler, type);
    begin_scope();

    consume(TokenType::TOKEN_LEFT_PAREN, "expect `(` after function name");
    consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after parameters");
    consume(TokenType::TOKEN_LEFT_BRACE, "expect `{` before function body");
    block();

    ObjFunction* function = end_compiler();
    emit_bytes(OpCode::OP_CONSTANT, current_chunk()->add_constant(function));
  }

  void fun_declaration() noexcept {
    u8_t global = parse_variable("expect function name");
    mark_initialized();
    function(FunctionType::TYPE_FUNCTION);
    define_variable(global);
  }

  void var_declaration() noexcept {
    u8_t global = parse_variable("expect variable name");

    if (match(TokenType::TOKEN_EQUAL)) {
      expression();
    }
    else {
      emit_byte(OpCode::OP_NIL);
    }
    consume(TokenType::TOKEN_SEMICOLON, "expect `;` after variable declaration");

    define_variable(global);
  }

  void expression_statement() noexcept {
    expression();
    consume(TokenType::TOKEN_SEMICOLON, "expect `;` after expression");
    emit_byte(OpCode::OP_POP);
  }

  void for_statement() noexcept {
    //          initializer clause
    //          condition expression <----.
    //    .---- OP_JUMP_IF_FALSE          |
    //    |     OP_POP                    |
    // .--+---- OP_JUMP                   |
    // |  |                               |
    // |  |     increment expression <----+---.
    // |  |     OP_POP                    |   |
    // |  |     OP_LOOP              -----*   |
    // |  |                                   |
    // `--+---> body statement                |
    //    |     OP_LOOP              ---------*
    //    `---> OP_POP
    //          continues ...

    begin_scope();
    consume(TokenType::TOKEN_LEFT_PAREN, "expect `(` after `for`.");
    if (match(TokenType::TOKEN_SEMICOLON)) {
      // no initializer ...
    }
    else if (match(TokenType::KEYWORD_VAR)) {
      var_declaration();
    }
    else {
      expression_statement();
    }

    int loop_start = as_type<int>(current_chunk()->codes_count());
    int exit_jump = -1;
    if (!match(TokenType::TOKEN_SEMICOLON)) {
      expression();
      consume(TokenType::TOKEN_SEMICOLON, "expect `;` after loop condition.");

      // jump out of the loop if the condition is false
      exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
      emit_byte(OpCode::OP_POP); // condition
    }

    if (!match(TokenType::TOKEN_RIGHT_PAREN)) {
      int body_jump = emit_jump(OpCode::OP_JUMP);
      int increment_start = as_type<int>(current_chunk()->codes_count());
      expression();
      emit_byte(OpCode::OP_POP);
      consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after for clauses");

      emit_loop(loop_start);
      loop_start = increment_start;
      patch_jump(body_jump);
    }

    statement();
    emit_loop(loop_start);

    if (exit_jump != -1) {
      patch_jump(exit_jump);
      emit_byte(OpCode::OP_POP); // condition
    }

    end_scope();
  }

  void if_statement() noexcept {
    //        condition expression
    //    .-- OP_JUMP_IF_FALSE
    //    |   OP_POP
    //    |
    //    |   then branch statement
    // .--+-- OP_JUMP
    // |  `-> OP_POP
    // |
    // |      else branch statement
    // `----> continues ...

    consume(TokenType::TOKEN_LEFT_PAREN, "expect `(` after `if`");
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after condition");

    int then_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_byte(OpCode::OP_POP);
    statement();

    int else_jump = emit_jump(OpCode::OP_JUMP);
    patch_jump(then_jump);
    emit_byte(OpCode::OP_POP);

    if (match(TokenType::KEYWORD_ELSE))
      statement();
    patch_jump(else_jump);
  }

  void print_statement() noexcept {
    expression();
    consume(TokenType::TOKEN_SEMICOLON, "expect `;` after value");
    emit_byte(OpCode::OP_PRINT);
  }

  void while_statement() noexcept {
    //      condition expression <--.
    // .--- OP_JUMP_IF_FALSE        |
    // |    OP_POP                  |
    // |                            |
    // |    body statement          |
    // |    OP_LOOP         --------* 
    // `--> OP_POP
    //      continues ...

    int loop_start = as_type<int>(current_chunk()->codes_count());

    consume(TokenType::TOKEN_LEFT_PAREN, "expect `(` after `while` keyword.");
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after condition.");

    int exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_byte(OpCode::OP_POP);
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OpCode::OP_POP);
  }

  void synchronize() noexcept {
    panic_mode_ = false;

    while (current_.type() != TokenType::TOKEN_EOF) {
      if (previous_.type() == TokenType::TOKEN_SEMICOLON)
        return;

      switch (current_.type()) {
      case TokenType::KEYWORD_CLASS:
      case TokenType::KEYWORD_FUN:
      case TokenType::KEYWORD_VAR:
      case TokenType::KEYWORD_FOR:
      case TokenType::KEYWORD_IF:
      case TokenType::KEYWORD_WHILE:
      case TokenType::KEYWORD_PRINT:
      case TokenType::KEYWORD_RETURN:
        return;
      default:
        break; // Do nothing ...
      }

      advance();
    }
  }
public:
  Parser(VM& vm, Scanenr& scanner) noexcept : vm_{vm}, scanner_{scanner} {}

  ObjFunction* compile() {
    Compiler compiler;
    init_compiler(&compiler, FunctionType::TYPE_SCRIPT);

    advance();
    while (!match(TokenType::TOKEN_EOF)) {
      declaration();
    }

    ObjFunction* function = end_compiler();
    return had_error_ ? nullptr : function;
  }
};

ObjFunction* GlobalCompiler::compile(VM& vm, const str_t& source) noexcept {
  Scanenr scanner(source);

  if (parser_ = new Parser{vm, scanner}; parser_ != nullptr) {
    ObjFunction* function = parser_->compile();
    delete parser_;
    parser_ = nullptr;

    return function;
  }
  return nullptr;
}

}
