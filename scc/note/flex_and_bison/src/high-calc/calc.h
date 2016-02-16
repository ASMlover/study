/*
 * Copyright (c) 2015 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __CALC_HEADER_H__
#define __CALC_HEADER_H__

extern int yylineno;
void yyerror(char* s, ...);

struct ast;
struct symlist;

struct symbol {
  /* 符号表 */
  char* name;
  double value;
  struct ast* func;
  struct symlist* syms;
};

#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol* lookup(char* sym);

struct symlist {
  /* 符号列表 */
  struct symbol* sym;
  struct symlist* next;
};
struct symlist* newsymlist(struct symbol* sym, struct symlist* next);
void symlistfree(struct symlist* sl);

/*
 * + - * / |
 * 0-7 比较操作符, 04等于, 02小于, 01大于
 * M 单目负号
 * L 表达式或语句列表
 * I IF
 * W WHILE
 * N 符号引用
 * = 赋值
 * S 符号列表
 * F 内置函数
 * C 用户函数
 */
enum bifs {
  /* 内置函数 */
  B_SQRT = 1,
  B_EXP,
  B_LOG,
  B_PRINT,
  B_EXIT,
};

struct ast {
  /* 抽象语法树 */
  int nodetype;
  struct ast* l;
  struct ast* r;
};

struct fncall {
  /* 内置函数 */
  int nodetype; /* F */
  struct ast* l;
  enum bifs functype;
};

struct ufncall {
  /* 用户函数 */
  int nodetype; /* C */
  struct ast* l;
  struct symbol* s;
};

struct flow {
  int nodetype; /* I or W */
  struct ast* cond;
  struct ast* tl; /* then or do */
  struct ast* el; /* else */
};

struct numval {
  int nodetype; /* K */
  double number;
};

struct symref {
  int nodetype; /* N */
  struct symbol* s;
};

struct symasgn {
  int nodetype; /* = */
  struct symbol* s;
  struct ast* v;
};

struct ast* newast(int nodetype, struct ast* l, struct ast* r);
struct ast* newcmp(int cmptype, struct ast* l, struct ast* r);
struct ast* newfunc(int functype, struct ast* l);
struct ast* newcall(struct symbol* s, struct ast* l);
struct ast* newref(struct symbol* s);
struct ast* newasgn(struct symbol* s, struct ast* l);
struct ast* newnum(double d);
struct ast* newflow(int nodetype, struct ast* cond,
    struct ast* tl, struct ast* el);

void dodef(struct symbol* s, struct symlist* syms, struct ast* stmts);
double eval(struct ast* t);
void treefree(struct ast* t);

#endif  /* __CALC_HEADER_H__ */
