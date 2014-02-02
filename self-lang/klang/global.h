/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __GLOBAL_HEADER_H__
#define __GLOBAL_HEADER_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"


#ifndef countof
# define countof(x)   (sizeof(x) / sizeof(*x))
#endif


typedef enum KL_Boolean {
  /* boolean type */
  BOOL_NO = 0, 
  BOOL_YES, 
} KL_Boolean;


enum KL_ValueType {
  VT_ERR = 0, 
  VT_BOOL,    /* boolean type */
  VT_INT,     /* int type */
  VT_REAL,    /* real type */
  VT_STR,     /* string type */
  VT_NIL,     /* nil type */
};

typedef struct KL_String {
  /* string */
  int         ref_count;
  char*       string;
  KL_Boolean  is_literal;
} KL_String;

typedef struct KL_Value {
  /* value */
  int val_type;
  union {
    KL_Boolean  bool_val;
    int         int_val;
    double      real_val;
    KL_String*  str_val;
  } value;
} KL_Value;

enum KL_ExprType {
  ET_ERR = 0, 
  ET_BOOL,      /* true/false */
  ET_INT,       /* int variable */
  ET_REAL,      /* real variable */
  ET_STR,       /* string variable */
  ET_ID,        /* identifier */
  ET_ASSIGN,    /* = */
  ET_ADD,       /* + */
  ET_SUB,       /* - */
  ET_MUL,       /* * */
  ET_DIV,       /* / */
  ET_MOD,       /* % */
  ET_EQ,        /* == */
  ET_NEQ,       /* <> */
  ET_GT,        /* > */
  ET_GE,        /* >= */
  ET_LT,        /* < */
  ET_LE,        /* <= */
  ET_AND,       /* && */
  ET_OR,        /* || */
  ET_MINUS,     /* - */
  ET_FUNC_CALL, /* call function */
  ET_NIL,       /* nil */
};

typedef struct KL_Expr KL_Expr;

typedef struct KL_ArgList {
  /* function call argument list */
  struct KL_ArgList*  next;
  KL_Expr*            expr;
} KL_ArgList;

typedef struct KL_AssignExpr {
  /* = expr */
  char*     variable;
  KL_Expr*  operand;
} KL_AssignExpr;

typedef struct KL_BinaryExpr {
  /* binary operator expression */
  KL_Expr* left;
  KL_Expr* right;
} KL_BinaryExpr;

typedef struct KL_FuncCallExpr {
  /* function call expression */
  char*       identifier;
  KL_ArgList* argument;
} KL_FuncCallExpr;


struct KL_Expr {
  /* expression */
  int expr_type;    /* type of expression */
  int lineno;       /* line number of this expression */
  union {
    KL_Boolean      bool_val;
    int             int_val;
    double          real_val;
    char*           str_val;
    char*           identifier;
    KL_AssignExpr   assign_expr;
    KL_BinaryExpr   binary_expr;
    KL_Expr*        minus_expr;
    KL_FuncCallExpr func_call_expr;
  } expr;
};

typedef struct KL_Stmt KL_Stmt;

typedef struct KL_StmtList {
  /* statement list */
  struct KL_StmtList* next;
  KL_Stmt*            stmt;
} KL_StmtList;

typedef struct KL_Block {
  /* block */
  KL_StmtList*  stmt_list;
} KL_Block;

typedef struct KL_IDList {
  /* identifier list */
  struct KL_IDList* next;
  char*             name;   /* name of identifier */
} KL_IDList;

typedef struct KL_GlobalStmt {
  /* global statement */
  KL_IDList* id_list;
} KL_GlobalStmt;

typedef struct KL_IfStmt {
  /* if statement */
  KL_Expr*  cond;
  KL_Block* then_block;
  KL_Block* else_block;
} KL_IfStmt;

typedef struct KL_WhileStmt {
  /* while statement */
  KL_Expr*  cond;
  KL_Block* block;
} KL_WhileStmt;


typedef struct KL_RetStmt {
  /* return statement */
  KL_Expr* ret_expr;
} KL_RetStmt;



enum KL_StmtType {
  ST_ERR = 0, 
  ST_EXPR,    /* expression statement */
  ST_GLOBAL,  /* global statement */
  ST_IF,      /* if statement */
  ST_WHILE,   /* while statement */
  ST_RET,     /* return statement */
  ST_BREAK,   /* break statement */
};

struct KL_Stmt {
  /* statement */
  int stmt_type;
  int lineno;
  union {
    KL_Expr*      expr_s;
    KL_GlobalStmt global_s;
    KL_IfStmt     if_s;
    KL_WhileStmt  while_s;
    KL_RetStmt    ret_s;
  } stmt;
};


typedef struct KL_ParamList {
  /* parameter list for function definition */
  struct KL_ParamList*  next;
  char*                 name;
} KL_ParamList;



enum KL_FuncType {
  FT_ERR = 0, 
  FT_DEFINE,  /* defined function by user */
  FT_NATIVE,  /* native function */
};


typedef struct KL_State KL_State;
typedef KL_Value (*KL_NativeFuncType)(
    KL_State* L, int nargs, KL_Value* args);

typedef struct KL_Function {
  /* function definition */
  struct KL_Function*   next;
  char*                 name;
  int                   func_type;
  union {
    struct {
      KL_ParamList*     param;
      KL_Block*         block;
    } define;

    struct {
      KL_NativeFuncType proc;
    } native;
  } func;
} KL_Function;


typedef struct KL_Variable {
  /* variable */
  struct KL_Variable* next;
  char*               name;
  KL_Value            value;
} KL_Variable;



enum KL_StmtResultType {
  SRT_ERR = 0, 
  SRT_NORMAL,   /* normal statement result */
  SRT_RETURN,   /* return statement result */
  SRT_BREAK,    /* break statement result */
};

typedef struct KL_StmtResult {
  /* statement result */
  int         type;
  union {
    KL_Value  value;
  } stmt_result;
} KL_StmtResult;


typedef struct KL_GlobalVariableRef {
  /* global variable reference */
  struct KL_GlobalVariableRef*  next;
  KL_Variable*                  variable;
} KL_GlobalVariableRef;


typedef struct KL_LocalEnv {
  /* local environment */
  KL_Variable*          variable;
  KL_GlobalVariableRef* global_variable;
} KL_LocalEnv;




/*
 * KL Interpreter for KL language.
 * It's main structure for KL language.
 */
struct KL_State {
  KL_MemStorage*  kl_storage;
  KL_MemStorage*  exec_storage;
  KL_Variable*    variable;
  KL_Function*    func_list;
  KL_StmtList*    stmt_list;
  int             lineno;
};


/* create functions */
extern void KL_function_def(const char* identifier, 
    KL_ParamList* param_list, KL_Block* block);
extern KL_ParamList* KL_create_param(const char* identifier);
extern KL_ParamList* KL_chain_param(
    KL_ParamList* list, const char* identifier);
extern KL_ArgList* KL_create_arg_list(KL_Expr* expr);
extern KL_ArgList* KL_chain_arg_list(KL_ArgList* list, KL_Expr* expr);
extern KL_StmtList* KL_create_stmt_list(KL_Stmt* stmt);
extern KL_StmtList* KL_chain_stmt_list(KL_StmtList* list, KL_Stmt* stmt);
extern KL_Expr* KL_alloc_expr(int expr_type);
extern KL_Expr* KL_create_assign_expr(
    const char* identifier, KL_Expr* oprand);
extern KL_Expr* KL_create_bin_expr(int oper, KL_Expr* left, KL_Expr* right);
extern KL_Expr* KL_create_minus_expr(KL_Expr* oprand);
extern KL_Expr* KL_create_identifier_expr(const char* identifier);
extern KL_Expr* KL_create_func_call_expr(
    const char* func_name, KL_ArgList* argument);
extern KL_Expr* KL_create_boolean_expr(KL_Boolean v);
extern KL_Expr* KL_create_nil_expr(void);
extern KL_Stmt* KL_create_global_stmt(KL_IDList* id_list);
extern KL_IDList* KL_create_global_identifier(const char* identifier);
extern KL_IDList* KL_chain_identifier(
    KL_IDList* list, const char* identifier);
extern KL_Stmt* KL_create_if_stmt( KL_Expr* cond, 
    KL_Block* then_block, KL_Block* else_block);
extern KL_Stmt* KL_create_while_stmt(KL_Expr* cond, KL_Block* block);
extern KL_Block* KL_create_block(KL_StmtList* stmt_list);
extern KL_Stmt* KL_create_expr_stmt(KL_Expr* expr);
extern KL_Stmt* KL_create_return_stmt(KL_Expr* expr);
extern KL_Stmt* KL_create_break_stmt(void);


/* string */
extern char* KL_create_identifier(const char* str);
extern void KL_open_string_literal(void);
extern void KL_add_string_literal(int letter);
extern void KL_reset_string_literal_buffer(void);
extern char* KL_close_string_literal(void);


/* execute module */
extern KL_StmtResult KL_exec_stmt_list(KL_State* L, 
    KL_LocalEnv* env, KL_StmtList* list);


/* eval module */
extern KL_Value KL_eval_bin_expr(KL_State* L, 
    KL_LocalEnv* env, int oper, KL_Expr* left, KL_Expr* right);
extern KL_Value KL_eval_minus_expr(KL_State* L, 
    KL_LocalEnv* env, KL_Expr* operand);
extern KL_Value KL_eval_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr);



/* string pool module */
extern KL_String* KL_literal_to_string(KL_State* L, const char* str);
extern void KL_refer_string(KL_String* str);
extern void KL_release_string(KL_String* str);
extern KL_String* KL_lookup_string(KL_State* L, const char* str);
extern KL_String* KL_create_string(KL_State* L, const char* str);



/* util module */
extern KL_State* KL_get_state(void);
extern void KL_set_state(KL_State* L);
extern void* KL_util_malloc(size_t size);
extern void* KL_util_exec_malloc(KL_State* L, size_t size);
extern KL_Variable* KL_lookup_local_variable(
    KL_LocalEnv* env, const char* identifier);
extern KL_Variable* KL_lookup_global_variable(
    KL_State* L, const char* identifier);
extern void KL_add_local_variable(KL_LocalEnv* env, 
    const char* identifier, KL_Value* value);
extern void KL_add_global_variable(KL_State* L, 
    const char* identifier, KL_Value* value);
extern KL_NativeFuncType KL_lookup_native_func(
    KL_State* L, const char* name);
extern KL_Function* KL_lookup_func(const char* name);
extern char* KL_get_oper_string(int expr_type);


#endif  /* __GLOBAL_HEADER_H__ */
