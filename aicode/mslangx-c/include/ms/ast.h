#ifndef MS_AST_H_
#define MS_AST_H_

#include <stddef.h>

#include "ms/buffer.h"
#include "ms/token.h"

typedef enum MsAstKind {
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
      MsToken left_brace;
      MsToken right_brace;
      const char *raw_start;
      size_t raw_length;
    } block;
  } as;
};

const char *ms_ast_kind_name(MsAstKind kind);
int ms_ast_dump(MsBuffer *buffer, const MsAstNode *node);

#endif