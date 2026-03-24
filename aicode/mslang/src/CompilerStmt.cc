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
#include "VM.hh"
#include "Memory.hh"

namespace ms {

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

    consume_semi("Expect ';' after variable declaration.");
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
    consume_semi("Expect ';' after variable declaration.");
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
    consume_semi("Expect ';' after variable declaration.");
    emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, val_reg, global));
    free_reg(val_reg);
  }
}

void Compiler::function(FunctionType type) noexcept {
  Compiler compiler(*ps_, type);
  compiler.begin_scope();

  consume(TokenType::TOKEN_LEFT_PAREN, "Expect '(' after function name.");
  bool seen_default = false;
  if (!check(TokenType::TOKEN_RIGHT_PAREN)) {
    do {
      // Rest parameter: ...name (must be last)
      if (match(TokenType::TOKEN_ELLIPSIS)) {
        compiler.function_->increment_arity();
        if (compiler.function_->arity() > 255)
          compiler.error_at_current("Can't have more than 255 parameters.");
        compiler.parse_variable("Expect rest parameter name.");
        compiler.mark_initialized();
        if (compiler.reg_top_ < static_cast<u8_t>(compiler.local_count_)) {
          compiler.reg_top_ = static_cast<u8_t>(compiler.local_count_);
          compiler.check_stack(compiler.reg_top_);
        }
        compiler.function_->set_has_rest_param(true);
        // rest param must be last
        break;
      }

      compiler.function_->increment_arity();
      if (compiler.function_->arity() > 255)
        compiler.error_at_current("Can't have more than 255 parameters.");
      compiler.parse_variable("Expect parameter name.");
      compiler.mark_initialized();
      if (compiler.reg_top_ < static_cast<u8_t>(compiler.local_count_)) {
        compiler.reg_top_ = static_cast<u8_t>(compiler.local_count_);
        compiler.check_stack(compiler.reg_top_);
      }

      // Default value: = expr (must be a simple constant)
      if (check(TokenType::TOKEN_EQUAL)) {
        advance(); // consume '='
        if (!seen_default) {
          compiler.function_->set_min_arity(compiler.function_->arity() - 1);
          compiler.function_->set_default_base(
              static_cast<int>(compiler.function_->chunk().constants().size()));
          seen_default = true;
        }
        // Parse default expression — emit as constant by evaluating literal
        // For simplicity, support literals: nil, true, false, number, string
        Value def_val;
        if (check(TokenType::TOKEN_NIL)) {
          advance(); def_val = Value();
        } else if (check(TokenType::TOKEN_TRUE)) {
          advance(); def_val = Value(true);
        } else if (check(TokenType::TOKEN_FALSE)) {
          advance(); def_val = Value(false);
        } else if (check(TokenType::TOKEN_INTEGER)) {
          advance();
          i64_t v = 0;
          for (char ch : ps_->previous.lexeme)
            if (ch >= '0' && ch <= '9') v = v * 10 + (ch - '0');
          def_val = Value(v);
        } else if (check(TokenType::TOKEN_NUMBER)) {
          advance();
          def_val = Value(std::stod(str_t(ps_->previous.lexeme)));
        } else if (check(TokenType::TOKEN_STRING)) {
          advance();
          strv_t sv = ps_->previous.lexeme;
          str_t s(sv.substr(1, sv.size() - 2)); // strip quotes
          def_val = Value(static_cast<Object*>(
              VM::get_instance().copy_string(s.data(), s.size())));
        } else {
          error("Default parameter must be a literal value (nil, bool, number, string).");
          advance();
        }
        compiler.function_->chunk().constants().push_back(def_val);
      } else {
        if (seen_default) {
          error("Non-default parameter after default parameter.");
        }
      }
    } while (match(TokenType::TOKEN_COMMA));
  }
  if (!seen_default) {
    compiler.function_->set_min_arity(compiler.function_->arity());
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
  // Detect fun* (generator) — star comes before the function name
  bool is_generator = match(TokenType::TOKEN_STAR);
  u16_t global = parse_variable("Expect function name.");
  mark_initialized();
  function(is_generator ? FunctionType::TYPE_GENERATOR : FunctionType::TYPE_FUNCTION);

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

void Compiler::enum_declaration() noexcept {
  // enum Name { VARIANT [= expr], ... }
  // Desugars to: create class, set static int fields for each variant
  consume(TokenType::TOKEN_IDENTIFIER, "Expect enum name.");
  Token enum_name = ps_->previous;
  u16_t name_constant = identifier_constant(ps_->previous);
  declare_variable();

  u8_t enum_reg;
  if (scope_depth_ > 0) {
    enum_reg = static_cast<u8_t>(local_count_ - 1);
    if (reg_top_ <= enum_reg) { reg_top_ = enum_reg + 1; check_stack(reg_top_); }
  } else {
    enum_reg = alloc_reg();
  }
  emit_instr(encode_ABx(OpCode::OP_CLASS, enum_reg, name_constant));

  consume(TokenType::TOKEN_LEFT_BRACE, "Expect '{' after enum name.");

  i64_t counter = 0;
  while (!check(TokenType::TOKEN_RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
    consume(TokenType::TOKEN_IDENTIFIER, "Expect variant name.");
    Token variant_tok = ps_->previous;
    u16_t variant_ki = identifier_constant(variant_tok);

    u8_t val_reg = alloc_reg();
    if (match(TokenType::TOKEN_EQUAL)) {
      // Explicit numeric value
      if (!check(TokenType::TOKEN_INTEGER) && !check(TokenType::TOKEN_NUMBER)) {
        error("Enum value must be an integer literal.");
        advance();
        counter = 0;
      } else {
        advance(); // consume the literal
        Token lit = ps_->previous;
        if (lit.type == TokenType::TOKEN_INTEGER) {
          // Parse integer literal
          i64_t v = 0;
          for (char ch : lit.lexeme) { if (ch >= '0' && ch <= '9') v = v * 10 + (ch - '0'); }
          counter = v;
        } else {
          counter = static_cast<i64_t>(std::stod(str_t(lit.lexeme)));
        }
      }
    }
    u16_t val_ki = static_cast<u16_t>(make_constant(Value(counter)));
    emit_instr(encode_ABx(OpCode::OP_LOADK, val_reg, val_ki));
    emit_instr(encode_ABC(OpCode::OP_STATICMETH, enum_reg,
                          static_cast<u8_t>(variant_ki), val_reg));
    free_reg(val_reg);
    counter++;

    if (!match(TokenType::TOKEN_COMMA)) break;
  }
  consume(TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after enum body.");

  if (scope_depth_ > 0) {
    mark_initialized();
  } else {
    emit_instr(encode_ABx(OpCode::OP_DEFGLOBAL, enum_reg, name_constant));
    free_reg(enum_reg);
  }
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
    // Skip spurious ASI-inserted `;` between methods
    while (match(TokenType::TOKEN_SEMICOLON)) {}
    if (check(TokenType::TOKEN_RIGHT_BRACE) || check(TokenType::TOKEN_EOF)) break;

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

  consume_semi("Expect ';' after import path.");

  u8_t path_reg = alloc_reg();
  emit_instr(encode_ABx(OpCode::OP_LOADK, path_reg, static_cast<u16_t>(ki)));
  emit_instr(encode_ABC(OpCode::OP_IMPORT, path_reg, 0, 0));
  free_reg(path_reg);
}

void Compiler::expression_statement() noexcept {
  expression();
  consume_semi("Expect ';' after expression.");
  // Free the register holding the result (no POP needed in register-based)
  u8_t reg = discharge(last_expr_);
  free_reg(reg);
}

void Compiler::print_statement() noexcept {
  expression();
  consume_semi("Expect ';' after value.");
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

  consume_semi("Expect ';' after 'break'.");
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

  consume_semi("Expect ';' after 'continue'.");
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
    consume_semi("Expect ';' after return value.");
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
  consume_semi("Expect ';' after throw value.");
  emit_instr(encode_ABC(OpCode::OP_THROW, reg, 0, 0));
  free_reg(reg);
}

void Compiler::yield_statement() noexcept {
  if (type_ != FunctionType::TYPE_GENERATOR) {
    error("Can't use 'yield' outside a generator function.");
    return;
  }

  u8_t val_reg;
  if (check(TokenType::TOKEN_SEMICOLON) || check(TokenType::TOKEN_EOF)) {
    // yield; — yield nil
    val_reg = alloc_reg();
    emit_instr(encode_ABC(OpCode::OP_LOADNIL, val_reg, val_reg, 0));
  } else {
    expression();
    val_reg = discharge(last_expr_);
  }
  consume_semi("Expect ';' after yield value.");
  emit_instr(encode_ABC(OpCode::OP_YIELD, val_reg, 0, 0));
  free_reg(val_reg);
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
    // Skip spurious ASI-inserted `;`
    while (match(TokenType::TOKEN_SEMICOLON)) {}
    if (check(TokenType::TOKEN_RIGHT_BRACE) || check(TokenType::TOKEN_EOF)) break;

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
  } else if (match(TokenType::TOKEN_YIELD)) {
    yield_statement();
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
  // Skip empty statements (e.g. spurious ASI-inserted `;` after `}`)
  while (match(TokenType::TOKEN_SEMICOLON)) {}
  if (check(TokenType::TOKEN_RIGHT_BRACE) || check(TokenType::TOKEN_EOF) ||
      check(TokenType::TOKEN_CASE) || check(TokenType::TOKEN_DEFAULT)) return;

  if (match(TokenType::TOKEN_ENUM)) {
    enum_declaration();
  } else if (match(TokenType::TOKEN_CLASS)) {
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

    consume_semi("Expect ';' after import statement.");
  } else {
    statement();
  }

  if (ps_->panic_mode) synchronize();
}

} // namespace ms
