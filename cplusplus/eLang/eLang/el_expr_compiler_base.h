// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_EXPR_COMPILER_BASE_HEADER_H__
#define __EL_EXPR_COMPILER_BASE_HEADER_H__

namespace el {

class ArrayExpr;
class BindExpr;
class BlockExpr;
class MessageExpr;
class NameExpr;
class NumberExpr;
class ObjectExpr;
class ReturnExpr;
class SelfExpr;
class SequenceExpr;
class SetExpr;
class StringExpr;
class UndefineExpr;
class VarExpr;
class Object;

interface ExprCompilerBase {
  virtual ~ExprCompilerBase(void) {}

  virtual void Visit(const ArrayExpr& expr, int dest) = 0;
  virtual void Visit(const BindExpr& expr, int dest) = 0;
  virtual void Visit(const BlockExpr& expr, int dest) = 0;
  virtual void Visit(const MessageExpr& expr, int dest) = 0;
  virtual void Visit(const NameExpr& expr, int dest) = 0;
  virtual void Visit(const NumberExpr& expr, int dest) = 0;
  virtual void Visit(const ObjectExpr& expr, int dest) = 0;
  virtual void Visit(const ReturnExpr& expr, int dest) = 0;
  virtual void Visit(const SelfExpr& expr, int dest) = 0;
  virtual void Visit(const SequenceExpr& expr, int dest) = 0;
  virtual void Visit(const SetExpr& expr, int dest) = 0;
  virtual void Visit(const StringExpr& expr, int dest) = 0;
  virtual void Visit(const UndefineExpr& expr, int dest) = 0;
  virtual void Visit(const VarExpr& expr, int dest) = 0;
};

}

#endif  // __EL_EXPR_COMPILER_BASE_HEADER_H__
