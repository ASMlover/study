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
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"

void yyerror(char* s, ...) {
  va_list ap;
  
  va_start(ap, s);
  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

static unsigned int symhash(char* sym) {
  unsigned int hash = 0;
  unsigned int c;

  while (c = *sym++)
    hash = hash * 9 ^ c;

  return hash;
}

struct symbol* lookup(char* sym) {
  struct symbol* sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH;

  while (--scount >= 0) {
    if (sp->name && !strcmp(sp->name, sym))
      return sp;

    if (!sp->name) {
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if (++sp >= symtab + NHASH)
      sp = symtab;
  }

  yyerror("symbol table overflow\n");
  abort();
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

struct ast* newcmp(int cmptype, struct ast* l, struct ast* r) {
  struct ast* t = (struct ast*)malloc(sizeof(struct ast));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = '0' + cmptype;
  t->l = l;
  t->r = r;
  return t;
}

struct ast* newfunc(int functype, struct ast* l) {
  struct fncall* t = (struct fncall*)malloc(sizeof(struct fncall));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = 'F';
  t->l = l;
  t->functype = functype;
  return (struct ast*)t;
}

struct ast* newcall(struct symbol* s, struct ast* l) {
  struct ufncall* t = (struct ufncall*)malloc(sizeof(struct ufncall));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = 'C';
  t->l = l;
  t->s = s;
  return (struct ast*)t;
}

struct ast* newref(struct symbol* s) {
  struct symref* t = (struct symref*)malloc(sizeof(struct symref));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = 'N';
  t->s = s;
  return (struct ast*)t;
}

struct ast* newasgn(struct symbol* s, struct ast* v) {
  struct symasgn* t = (struct symasgn*)malloc(sizeof(struct symasgn));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = '=';
  t->s = s;
  t->v = v;
  return (struct ast*)t;
}

struct ast* newflow(int nodetype,
    struct ast* cond, struct ast* tl, struct ast* el) {
  struct flow* t = (struct flow*)malloc(sizeof(struct flow));
  if (!t) {
    yyerror("out of memory");
    exit(0);
  }

  t->nodetype = nodetype;
  t->cond = cond;
  t->tl = tl;
  t->el = el;
  return (struct ast*)t;
}

void treefree(struct ast* t) {
  switch (t->nodetype) {
  case '+': case '-': case '*': case '/':
  case '1': case '2': case '3': case '4': case '5': case '6':
  case 'L':
    treefree(t->r);
  case '|':
  case 'M': case 'C': case 'F':
    treefree(t->l);
  case 'K': case 'N':
    break;
  case '=':
    free(((struct symasgn*)t)->v);
    break;
  case 'I': case 'W':
    free(((struct flow*)t)->cond);
    if (((struct flow*)t)->tl)
      free(((struct flow*)t)->tl);
    if (((struct flow*)t)->el)
      free(((struct flow*)t)->el);
    break;
  default:
    fprintf(stderr, "internal error: free bad node %c\n", t->nodetype);
  }

  free(t);
}

struct symlist* newsymlist(struct symbol* sym, struct symlist* next) {
  struct symlist* sl = (struct symlist*)malloc(sizeof(struct symlist));
  if (!sl) {
    yyerror("out of memory");
    exit(0);
  }

  sl->sym = sym;
  sl->next = next;
  return sl;
}

void symlistfree(struct symlist* sl) {
  struct symlist* n;
  while (sl) {
    n = sl->next;
    free(sl);
    sl = n;
  }
}

void dodef(struct symbol* name, struct symlist* syms, struct ast* func) {
  if (name->syms)
    symlistfree(name->syms);
  if (name->func)
    treefree(name->func);
  name->syms = syms;
  name->func = func;
}

static double callbuiltin(struct fncall* f) {
  enum bifs functype = f->functype;
  double v = eval(f->l);

  switch (functype) {
  case B_SQRT:
    return sqrt(v);
  case B_EXP:
    return exp(v);
  case B_LOG:
    return log(v);
  case B_PRINT:
    fprintf(stdout, "= %4.4g\n", v);
    return v;
  case B_EXIT:
    fprintf(stdout, "bye bye!!!\n");
    exit(v);
  default:
    yyerror("unknown built-in function %d", functype);
    return 0;
  }
}

static double calluser(struct ufncall* f) {
  struct symbol* fn = f->s;
  struct symlist* sl;
  struct ast* args = f->l;
  double* oldval;
  double* newval;
  double v;
  int nargs;
  int i;

  if (!fn->func) {
    yyerror("call to undefined function: %s", fn->name);
    return 0;
  }

  sl = fn->syms;
  for (nargs = 0; sl; sl = sl->next)
    ++nargs;

  oldval = (double*)malloc(nargs * sizeof(double));
  newval = (double*)malloc(nargs * sizeof(double));
  if (!oldval || !newval) {
    yyerror("out of space in %s", fn->name);
    return 0;
  }

  for (i = 0; i < nargs; ++i) {
    if (!args) {
      yyerror("too few args in call to %s", fn->name);
      free(oldval);
      free(newval);
      return 0;
    }

    if (args->nodetype == 'L') {
      newval[i] = eval(args->l);
      args = args->r;
    }
    else {
      newval[i] = eval(args);
      args = NULL;
    }
  }

  sl = fn->syms;
  for (i = 0; i < nargs; ++i) {
    struct symbol* s = sl->sym;
    oldval[i] = s->value;
    s->value = newval[i];
    sl = sl->next;
  }
  free(newval);

  v = eval(fn->func);
  sl = fn->syms;
  for (i = 0; i < nargs; ++i) {
    struct symbol* s = sl->sym;
    s->value = oldval[i];
    sl = sl->next;
  }
  free(oldval);

  return v;
}

double eval(struct ast* t) {
  double v;

  if (!t) {
    yyerror("internal error, null eval");
    return 0;
  }

  switch (t->nodetype) {
  case 'K': v = ((struct numval*)t)->number; break;
  case 'N': v = ((struct symref*)t)->s->value; break;
  case '=':
    v = ((struct symasgn*)t)->s->value = eval(((struct symasgn*)t)->v);
    break;
  case '+': v = eval(t->l) + eval(t->r); break;
  case '-': v = eval(t->l) - eval(t->r); break;
  case '*': v = eval(t->l) * eval(t->r); break;
  case '/': v = eval(t->l) / eval(t->r); break;
  case '|': v = fabs(eval(t->l)); break;
  case 'M': v = -eval(t->l); break;
  case '1': v = (eval(t->l) > eval(t->r)) ? 1 : 0; break;
  case '2': v = (eval(t->l) < eval(t->r)) ? 1 : 0; break;
  case '3': v = (eval(t->l) != eval(t->r)) ? 1 : 0; break;
  case '4': v = (eval(t->l) == eval(t->r)) ? 1 : 0; break;
  case '5': v = (eval(t->l) >= eval(t->r)) ? 1 : 0; break;
  case '6': v = (eval(t->l) <= eval(t->r)) ? 1 : 0; break;
  case 'I':
    if (eval(((struct flow*)t)->cond) != 0) {
      if (((struct flow*)t)->tl)
        v = eval(((struct flow*)t)->tl);
      else
        v = 0;
    }
    else {
      if (((struct flow*)t)->el)
        v = eval(((struct flow*)t)->el);
      else
        v = 0;
    }
    break;
  case 'W':
    v = 0;
    if (((struct flow*)t)->tl) {
      while (eval(((struct flow*)t)->cond) != 0)
        v = eval(((struct flow*)t)->tl);
    }
    break;
  case 'L': eval(t->l); v = eval(t->r); break;
  case 'F': v = callbuiltin((struct fncall*)t); break;
  case 'C': v = calluser((struct ufncall*)t); break;
  default:
    fprintf(stderr, "internal error: bad node %c\n", t->nodetype);
  }
  return v;
}

int main(int argc, char* argv[]) {
  fprintf(stdout, "> ");
  return yyparse();
}
