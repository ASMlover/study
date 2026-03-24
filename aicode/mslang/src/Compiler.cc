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
#include "CompilerImpl.hh"
#include "Debug.hh"
#include "Optimize.hh"

namespace ms {


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
  if (type == FunctionType::TYPE_GENERATOR) {
    function_->set_is_generator(true);
    local.name.lexeme = "";
  } else if (type != FunctionType::TYPE_FUNCTION) {
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

// Consume a statement-ending semicolon, or accept `}` / EOF as an implicit
// terminator so that `{ stmt }` on one line works without a trailing `;`.
void Compiler::consume_semi(strv_t message) noexcept {
  if (ps_->current.type == TokenType::TOKEN_SEMICOLON) {
    advance();
    return;
  }
  if (ps_->current.type == TokenType::TOKEN_RIGHT_BRACE ||
      ps_->current.type == TokenType::TOKEN_EOF) {
    return;  // implicit terminator — do not consume
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
  if (!ps.had_error && function) peephole_optimize(function);
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
  if (!ps.had_error && function) peephole_optimize(function);
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
