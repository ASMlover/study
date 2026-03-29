#ifndef MS_AST_H_
#define MS_AST_H_

#include <stddef.h>

#include "ms/buffer.h"
#include "ms/token.h"

typedef enum MsAstKind {
  MS_AST_PROGRAM,
  MS_AST_VAR_DECL,
  MS_AST_FUNCTION_DECL,
  MS_AST_CLASS_DECL,
  MS_AST_IMPORT_STMT,
  MS_AST_FROM_IMPORT_STMT,
  MS_AST_EXPR_STMT,
  MS_AST_PRINT_STMT,
  MS_AST_RETURN_STMT,
  MS_AST_BREAK_STMT,
  MS_AST_CONTINUE_STMT,
  MS_AST_IF_STMT,
  MS_AST_WHILE_STMT,
  MS_AST_FOR_STMT,
  MS_AST_LITERAL,
  MS_AST_VARIABLE,
  MS_AST_SELF,
  MS_AST_SUPER,
  MS_AST_UNARY,
  MS_AST_BINARY,
  MS_AST_LOGICAL,
  MS_AST_ASSIGN,
  MS_AST_CALL,
  MS_AST_PROPERTY,
  MS_AST_INDEX,
  MS_AST_LIST,
  MS_AST_TUPLE,
  MS_AST_MAP,
  MS_AST_FUNCTION,
  MS_AST_BLOCK
} MsAstKind;

typedef struct MsAstNode MsAstNode;

typedef struct MsTokenArray {
  size_t count;
  MsToken *items;
} MsTokenArray;

typedef struct MsAstNodeArray {
  size_t count;
  MsAstNode **items;
} MsAstNodeArray;

typedef struct MsAstMapEntry {
  MsAstNode *key;
  MsAstNode *value;
} MsAstMapEntry;

typedef struct MsAstMapEntryArray {
  size_t count;
  MsAstMapEntry *items;
} MsAstMapEntryArray;

struct MsAstNode {
  MsAstKind kind;
  size_t node_id;
  int line;
  int column;
  int end_column;
  union {
    struct {
      MsAstNodeArray declarations;
    } program;
    struct {
      MsToken name;
      MsAstNode *initializer;
    } var_decl;
    struct {
      MsToken name;
      MsTokenArray parameters;
      MsAstNode *body;
    } function_decl;
    struct {
      MsToken name;
      MsToken superclass;
      MsAstNodeArray methods;
    } class_decl;
    struct {
      MsTokenArray path;
      MsToken alias;
    } import_stmt;
    struct {
      MsTokenArray path;
      MsToken name;
      MsToken alias;
    } from_import_stmt;
    struct {
      MsAstNode *expression;
    } expr_stmt;
    struct {
      MsAstNode *expression;
    } print_stmt;
    struct {
      MsToken keyword;
      MsAstNode *value;
    } return_stmt;
    struct {
      MsToken keyword;
    } break_stmt;
    struct {
      MsToken keyword;
    } continue_stmt;
    struct {
      MsAstNode *condition;
      MsAstNode *then_branch;
      MsAstNode *else_branch;
    } if_stmt;
    struct {
      MsAstNode *condition;
      MsAstNode *body;
    } while_stmt;
    struct {
      MsAstNode *initializer;
      MsAstNode *condition;
      MsAstNode *increment;
      MsAstNode *body;
    } for_stmt;
    struct {
      MsToken token;
    } literal;
    struct {
      MsToken name;
    } variable;
    struct {
      MsToken keyword;
    } self_expr;
    struct {
      MsToken keyword;
      MsToken method;
    } super_expr;
    struct {
      MsToken op;
      MsAstNode *operand;
    } unary;
    struct {
      MsAstNode *left;
      MsToken op;
      MsAstNode *right;
    } binary;
    struct {
      MsAstNode *left;
      MsToken op;
      MsAstNode *right;
    } logical;
    struct {
      MsAstNode *target;
      MsAstNode *value;
    } assign;
    struct {
      MsAstNode *callee;
      MsAstNodeArray arguments;
    } call;
    struct {
      MsAstNode *object;
      MsToken name;
    } property;
    struct {
      MsAstNode *object;
      MsAstNode *index;
    } index;
    struct {
      MsAstNodeArray elements;
    } list;
    struct {
      MsAstNodeArray elements;
    } tuple;
    struct {
      MsAstMapEntryArray entries;
    } map;
    struct {
      MsTokenArray parameters;
      MsAstNode *body;
    } function;
    struct {
      MsAstNodeArray statements;
    } block;
  } as;
};

const char *ms_ast_kind_name(MsAstKind kind);
int ms_ast_dump(MsBuffer *buffer, const MsAstNode *node);

#endif