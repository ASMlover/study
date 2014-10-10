// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __EXPR_HEADER_H__
#define __EXPR_HEADER_H__

class BreakExpr;
class ContinueExpr;
typedef std::shared_ptr<FILE> FilePtr;
class Expr : private UnCopyable {
public:
  typedef std::shared_ptr<BreakExpr>                BreakPtr;
  typedef std::shared_ptr<ContinueExpr>             ContinuePtr;
  typedef std::unordered_map<std::string, uint32_t> VariableMap;
  typedef std::stack<std::list<BreakPtr> >          BreakListStack;
  typedef std::stack<std::list<ContinuePtr> >       ContinueListStack;
  typedef std::stack<std::list<uint32_t> >          UintListStack;
protected:
  static uint32_t    var_index_;
  static VariableMap variables_;

  static BreakListStack    end_;
  static ContinueListStack start_;
  static UintListStack     scope_;

  void PushResolve(void);
  void Resolve(FilePtr& out, uint32_t start_addr, uint32_t end_addr);
public:
  typedef std::shared_ptr<Expr> ExprPtr;
  ExprPtr left_;
  ExprPtr right_;
public:
  Expr(void);
  virtual ~Expr(void);

  static Expr& GetSingleton(void);

  virtual void Execute(FilePtr& out) = 0;

  static OpCode GetType(const std::string& token);
  static void Reset(void);
  void EvalChildren(FilePtr& out);
};

typedef std::shared_ptr<Expr> ExprPtr;

class BlockExpr : public Expr {
  typedef std::list<ExprPtr> ExprList;
public:
  ExprList blocks_;

  virtual ~BlockExpr(void);
  virtual void Execute(FilePtr& out);
};

class ValueExpr : public Expr {
public:
  std::string value_;

  virtual void Execute(FilePtr& out);
};

class VariableExpr : public Expr {
public:
  std::string name_;

  virtual void Execute(FilePtr& out);
};

class ArrayExpr : public Expr {
public:
  std::vector<ExprPtr> values_;

  virtual ~ArrayExpr(void);
  virtual void Execute(FilePtr& out);
};

class ExitExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class AssignExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class PrintExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class ReadExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class IfExpr : public Expr {
public:
  ExprPtr else_;

  IfExpr(void);
  virtual ~IfExpr(void);

  virtual void Execute(FilePtr& out);
};

class WhileExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class DoExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class ForExpr : public Expr {
public:
  ExprPtr assign_;
  ExprPtr action_;

  ForExpr(void);
  virtual ~ForExpr(void);

  virtual void Execute(FilePtr& out);
};

class BreakExpr : public Expr {
protected:
  uint32_t offset_;
public:
  virtual void Execute(FilePtr& out);
  void Resolve(FilePtr& out, uint32_t addr);
};

class ContinueExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class AndExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class OrExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class EqExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class NeExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class NotExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class LtExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class GtExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class LeExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class GeExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class AddExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class SubExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class MulExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class DivExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class ModExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

class PowExpr : public Expr {
public:
  virtual void Execute(FilePtr& out);
};

#endif  // __EXPR_HEADER_H__
