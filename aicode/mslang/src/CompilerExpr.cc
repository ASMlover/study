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
#include "CompilerImpl.hh"

namespace ms {


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
  ObjString* prop_name = as_string(current_chunk().constant_at(name_ki));

  // Object is in last_expr_
  u8_t obj_reg = discharge(last_expr_);

  // Emit helpers: encode IC slot directly in C (GETPROP/INVOKE) or B (SETPROP).
  // C/B == 0xFF is a fallback sentinel: EXTRAARG follows with ic_slot as u16_t.
  auto emit_getprop = [&](u8_t dest, u8_t obj) noexcept {
    sz_t ic_slot = function_->add_ic();
    function_->ic_at(ic_slot).name = prop_name;
    if (ic_slot <= 0xFE) {
      emit_instr(encode_ABC(OpCode::OP_GETPROP, dest, obj, static_cast<u8_t>(ic_slot)));
    } else {
      emit_instr(encode_ABC(OpCode::OP_GETPROP, dest, obj, 0xFF));
      emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    }
  };
  auto emit_setprop = [&](u8_t obj, u8_t val) noexcept {
    sz_t ic_slot = function_->add_ic();
    function_->ic_at(ic_slot).name = prop_name;
    if (ic_slot <= 0xFE) {
      emit_instr(encode_ABC(OpCode::OP_SETPROP, obj, static_cast<u8_t>(ic_slot), val));
    } else {
      emit_instr(encode_ABC(OpCode::OP_SETPROP, obj, 0xFF, val));
      emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    }
  };

  if (can_assign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    u8_t val_reg = discharge(last_expr_);
    emit_setprop(obj_reg, val_reg);
    free_reg(val_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else if (match(TokenType::TOKEN_LEFT_PAREN)) {
    // Method invocation: INVOKE A=base, B=argc, C=ic_slot
    u8_t base = obj_reg;
    if (base < static_cast<u8_t>(local_count_)) {
      base = alloc_reg();
      emit_instr(encode_ABC(OpCode::OP_MOVE, base, obj_reg, 0));
    }
    u8_t arg_count = argument_list(base);
    sz_t ic_slot = function_->add_ic();
    function_->ic_at(ic_slot).name = prop_name;
    if (ic_slot <= 0xFE) {
      emit_instr(encode_ABC(OpCode::OP_INVOKE, base, arg_count, static_cast<u8_t>(ic_slot)));
    } else {
      emit_instr(encode_ABC(OpCode::OP_INVOKE, base, arg_count, 0xFF));
      emit_instr(encode_ABx(OpCode::OP_EXTRAARG, 0, static_cast<u16_t>(ic_slot)));
    }
    reg_top_ = base + 1;
    last_expr_ = {ExprDesc::VREG, base};
  } else if (can_assign && is_compound_op(ps_->current.type)) {
    // Compound assignment: obj.prop op= expr
    OpCode op = compound_op(ps_->current.type);
    advance();
    u8_t prop_reg = alloc_reg();
    emit_getprop(prop_reg, obj_reg);
    expression();
    u8_t rk_right = to_rk(last_expr_);
    emit_instr(encode_ABC(op, prop_reg, prop_reg, rk_right));
    if (!is_rk_const(rk_right)) free_reg(rk_right);
    emit_setprop(obj_reg, prop_reg);
    free_reg(prop_reg);
    last_expr_ = {ExprDesc::VREG, obj_reg};
  } else {
    // GET_PROPERTY: R(dest) := R(obj).prop
    u8_t dest = alloc_reg();
    emit_getprop(dest, obj_reg);
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

} // namespace ms
