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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "calc.h"

void yyerror(char* s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

struct ast* newast(int nodetype, struct ast* l, struct ast* r) {
  struct ast* t = (struct ast*)malloc(sizeof(struct ast));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }
  
  t->nodetype = nodetype;
  t->l = l;
  t->r = r;
  return t;
}

struct ast* newnum(double d) {
  struct numval* t = (struct numval*)malloc(sizeof(struct numval));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }
  t->nodetype = 'K';
  t->number = d;
  return (struct ast*)t;
}

double eval(struct ast* t) {
  double v;

  switch (t->nodetype) {
  case 'K': v = ((struct numval*)t)->number; break;
  case '+': v = eval(t->l) + eval(t->r); break;
  case '-': v = eval(t->l) - eval(t->r); break;
  case '*': v = eval(t->l) * eval(t->r); break;
  case '/': v = eval(t->l) / eval(t->r); break;
  case '|': v = eval(t->l); if (v < 0) v = -v; break;
  case 'M': v = -eval(t->l); break;
  default: fprintf(stdout, "internal error: bad node %c\n", t->nodetype);
  }

  return v;
}

void treefree(struct ast* t) {
  switch (t->nodetype) {
  case '+':
  case '-':
  case '*':
  case '/':
    treefree(t->r);
  case '|':
  case 'M':
    treefree(t->l);
  case 'K':
    free(t);
    break;
  default:
    fprintf(stdout, "internal error: free bad node %c\n", t->nodetype);
  }
}

int main(int argc, char* argv[]) {
  fprintf(stdout, "> ");
  return yyparse();
}
