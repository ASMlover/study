// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Parser.hh"

namespace ms {

Value LiteralExpr::accept(Interpreter& Interpreter) const { return 0; }
Value GroupingExpr::accept(Interpreter& Interpreter) const { return 0; }
Value UnaryExpr::accept(Interpreter& Interpreter) const { return 0; }
Value BinayExpr::accept(Interpreter& Interpreter) const { return 0; }
Value LogicalExpr::accept(Interpreter& Interpreter) const { return 0; }
Value VariableExpr::accept(Interpreter& Interpreter) const { return 0; }
Value AssignExpr::accept(Interpreter& Interpreter) const { return 0; }
Value CallExpr::accept(Interpreter& Interpreter) const { return 0; }
Value GetExpr::accept(Interpreter& Interpreter) const { return 0; }
Value SetExpr::accept(Interpreter& Interpreter) const { return 0; }
Value ThisExpr::accept(Interpreter& Interpreter) const { return 0; }
Value SuperExpr::accept(Interpreter& Interpreter) const { return 0; }
void ExpressionStmt::accept(Interpreter& Interpreter) const {}
void PrintStmt::accept(Interpreter& Interpreter) const {}
void VarStmt::accept(Interpreter& Interpreter) const {}
void BlockStmt::accept(Interpreter& Interpreter) const {}
void IfStmt::accept(Interpreter& Interpreter) const {}
void WhileStmt::accept(Interpreter& Interpreter) const {}
void FunctionStmt::accept(Interpreter& Interpreter) const {}
void ClassStmt::accept(Interpreter& Interpreter) const {}
void ImportStmt::accept(Interpreter& Interpreter) const {}

}
