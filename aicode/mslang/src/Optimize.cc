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
#include <vector>
#include "Optimize.hh"
#include "Chunk.hh"
#include "Value.hh"

namespace ms {

// Build jump-target bitmap: instructions that are branch targets must not be
// removed or merged with a predecessor.
static std::vector<bool> build_jump_targets(const std::vector<Instruction>& code) noexcept {
  sz_t n = code.size();
  std::vector<bool> is_target(n, false);

  for (sz_t pc = 0; pc < n; ++pc) {
    OpCode op = decode_op(code[pc]);
    switch (op) {
    case OpCode::OP_JMP:
    case OpCode::OP_TRY:
    case OpCode::OP_FORITER: {
      int dest = static_cast<int>(pc) + 1 + decode_sBx(code[pc]);
      if (dest >= 0 && static_cast<sz_t>(dest) < n)
        is_target[static_cast<sz_t>(dest)] = true;
      break;
    }
    case OpCode::OP_TEST:
    case OpCode::OP_TESTSET:
      // These skip the next instruction on the condition
      if (pc + 2 < n) is_target[pc + 2] = true;
      break;
    default:
      break;
    }
  }
  return is_target;
}

static void optimize_function(ObjFunction* fn) noexcept {
  auto& code = fn->chunk().code();
  auto& constants = fn->chunk().constants();
  sz_t n = code.size();
  if (n == 0) return;

  auto is_target = build_jump_targets(code);

  bool changed = true;
  while (changed) {
    changed = false;
    n = code.size(); // may change after P2 coalescing (but we don't shrink, only NOP)

    for (sz_t pc = 0; pc < n; ++pc) {
      OpCode op = decode_op(code[pc]);

      // P1 — Redundant MOVE: MOVE R(A), R(B) where A == B → NOP
      if (op == OpCode::OP_MOVE) {
        u8_t A = decode_A(code[pc]);
        u8_t B = decode_B(code[pc]);
        if (A == B) {
          code[pc] = encode_ABC(OpCode::OP_NOP, 0, 0, 0);
          changed = true;
          continue;
        }
      }

      // P2 — LOADNIL coalescing: consecutive LOADNIL of adjacent regs
      if (op == OpCode::OP_LOADNIL && pc + 1 < n) {
        sz_t next = pc + 1;
        // skip NOPs
        while (next < n && decode_op(code[next]) == OpCode::OP_NOP) ++next;
        if (next < n && !is_target[next] && decode_op(code[next]) == OpCode::OP_LOADNIL) {
          u8_t A0 = decode_A(code[pc]);
          u8_t B0 = decode_B(code[pc]); // range: A0..A0+B0
          u8_t A1 = decode_A(code[next]);
          u8_t B1 = decode_B(code[next]);
          // Check adjacency: A1 == A0+B0+1
          if (A1 == static_cast<u8_t>(A0 + B0 + 1)) {
            // Merge: LOADNIL A0, B0+1+B1
            code[pc] = encode_ABC(OpCode::OP_LOADNIL, A0,
                static_cast<u8_t>(B0 + 1 + B1), 0);
            code[next] = encode_ABC(OpCode::OP_NOP, 0, 0, 0);
            changed = true;
            continue;
          }
        }
      }

      // P3 — Dead code after unconditional exit (RETURN or THROW)
      if ((op == OpCode::OP_RETURN || op == OpCode::OP_THROW) && pc + 1 < n) {
        sz_t next = pc + 1;
        while (next < n && !is_target[next]
               && decode_op(code[next]) != OpCode::OP_NOP) {
          code[next] = encode_ABC(OpCode::OP_NOP, 0, 0, 0);
          changed = true;
          ++next;
        }
        continue;
      }

      // P4 — LOADK + NEG folding: LOADK R(A), K(i) followed by NEG R(A), R(A)
      if (op == OpCode::OP_LOADK && pc + 1 < n) {
        sz_t next = pc + 1;
        while (next < n && decode_op(code[next]) == OpCode::OP_NOP) ++next;
        if (next < n && !is_target[next]
            && decode_op(code[next]) == OpCode::OP_NEG) {
          u8_t A = decode_A(code[pc]);
          u16_t ki = decode_Bx(code[pc]);
          u8_t negA = decode_A(code[next]);
          u8_t negB = decode_B(code[next]);
          if (negA == A && negB == A && ki < static_cast<u16_t>(constants.size())) {
            const Value& kval = constants[ki];
            Value negated;
            bool folded = false;
            if (kval.is_integer()) {
              negated = Value(static_cast<i64_t>(-kval.as_integer()));
              folded = true;
            } else if (kval.is_double()) {
              negated = Value(-kval.as_number());
              folded = true;
            }
            if (folded) {
              // Add negated constant to pool
              u16_t new_ki = static_cast<u16_t>(constants.size());
              constants.push_back(negated);
              code[pc] = encode_ABx(OpCode::OP_LOADK, A, new_ki);
              code[next] = encode_ABC(OpCode::OP_NOP, 0, 0, 0);
              changed = true;
              continue;
            }
          }
        }
      }

      // P5 — MOVE + RETURN tail merge: MOVE R(A), R(B) followed by RETURN R(A)
      if (op == OpCode::OP_MOVE && pc + 1 < n) {
        sz_t next = pc + 1;
        while (next < n && decode_op(code[next]) == OpCode::OP_NOP) ++next;
        if (next < n && !is_target[next]
            && decode_op(code[next]) == OpCode::OP_RETURN) {
          u8_t moveA = decode_A(code[pc]);
          u8_t moveB = decode_B(code[pc]);
          u8_t retA  = decode_A(code[next]);
          u8_t retB  = decode_B(code[next]);
          if (retA == moveA) {
            // Replace RETURN R(A) with RETURN R(B)
            code[next] = encode_ABC(OpCode::OP_RETURN, moveB, retB, 0);
            code[pc]   = encode_ABC(OpCode::OP_NOP, 0, 0, 0);
            changed = true;
            continue;
          }
        }
      }
    }
  }
}

void peephole_optimize(ObjFunction* fn) noexcept {
  if (fn == nullptr) return;

  optimize_function(fn);

  // Recurse into nested closures stored as constants
  for (auto& val : fn->chunk().constants()) {
    if (val.is_object() && val.as_object()->type() == ObjectType::OBJ_FUNCTION) {
      peephole_optimize(as_obj<ObjFunction>(val.as_object()));
    }
  }
}

} // namespace ms
