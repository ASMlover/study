#include "ms/parser.h"

#include <string.h>

#include "ms/buffer.h"

#include "parser_internal.h"

MsToken ms_parser_make_eof_token(const MsParser *parser) {
  MsToken token;

  token.kind = MS_TOKEN_EOF;
  token.start = parser->lexer.current;
  token.length = 0;
  token.line = parser->lexer.line;
  token.column = parser->lexer.column;
  token.end_column = parser->lexer.column;
  return token;
}

void ms_parser_advance(MsParser *parser) {
  parser->previous = parser->current;
  parser->current = ms_lexer_next(&parser->lexer);
}

int ms_parser_check(const MsParser *parser, MsTokenKind kind) {
  return parser->current.kind == kind;
}

int ms_parser_match(MsParser *parser, MsTokenKind kind) {
  if (!ms_parser_check(parser, kind)) {
    return 0;
  }

  ms_parser_advance(parser);
  return 1;
}

void ms_parser_skip_newlines(MsParser *parser) {
  while (ms_parser_match(parser, MS_TOKEN_NEWLINE)) {
  }
}

void ms_parser_append_diagnostic(MsParser *parser,
                                 const char *code,
                                 const char *message,
                                 int line,
                                 int column,
                                 int length) {
  MsDiagnostic diagnostic;

  if (parser->diagnostics == NULL) {
    parser->had_error = 1;
    return;
  }

  diagnostic.phase = "parse";
  diagnostic.code = code;
  diagnostic.message = message;
  diagnostic.span.file = parser->file;
  diagnostic.span.line = line;
  diagnostic.span.column = column;
  diagnostic.span.length = length > 0 ? length : 1;
  ms_diag_list_append(parser->diagnostics, &diagnostic);
  parser->had_error = 1;
}

void ms_parser_error_at_token(MsParser *parser,
                              MsToken token,
                              const char *code,
                              const char *message) {
  int length = token.length > 0 ? (int) token.length : 1;

  ms_parser_append_diagnostic(parser,
                              code,
                              message,
                              token.line,
                              token.column,
                              length);
}

void ms_parser_error_at_node(MsParser *parser,
                             const MsAstNode *node,
                             const char *code,
                             const char *message) {
  ms_parser_append_diagnostic(parser,
                              code,
                              message,
                              node->line,
                              node->column,
                              node->end_column - node->column + 1);
}

int ms_parser_expect(MsParser *parser,
                     MsTokenKind kind,
                     const char *code,
                     const char *message,
                     MsToken *out_token) {
  if (!ms_parser_check(parser, kind)) {
    ms_parser_error_at_token(parser, parser->current, code, message);
    return 0;
  }

  if (out_token != NULL) {
    *out_token = parser->current;
  }
  ms_parser_advance(parser);
  return 1;
}

MsAstNode *ms_parser_alloc_node(MsParser *parser,
                                MsAstKind kind,
                                int line,
                                int column,
                                int end_column) {
  MsAstNode *node = ms_arena_alloc(parser->arena,
                                   sizeof(*node),
                                   _Alignof(MsAstNode));

  if (node == NULL) {
    ms_parser_append_diagnostic(parser,
                                "MS2000",
                                "out of memory while building AST",
                                line,
                                column,
                                end_column - column + 1);
    return NULL;
  }

  memset(node, 0, sizeof(*node));
  node->kind = kind;
  node->node_id = ++parser->next_node_id;
  node->line = line;
  node->column = column;
  node->end_column = end_column;
  return node;
}

int ms_parser_copy_node_array(MsParser *parser,
                              const MsBuffer *buffer,
                              MsAstNodeArray *out_array) {
  size_t count = buffer->length / sizeof(MsAstNode *);

  out_array->count = count;
  out_array->items = NULL;
  if (count == 0) {
    return 1;
  }

  out_array->items = ms_arena_alloc(parser->arena,
                                    buffer->length,
                                    _Alignof(MsAstNode *));
  if (out_array->items == NULL) {
    return 0;
  }

  memcpy(out_array->items, buffer->data, buffer->length);
  return 1;
}

int ms_parser_copy_token_array(MsParser *parser,
                               const MsBuffer *buffer,
                               MsTokenArray *out_array) {
  size_t count = buffer->length / sizeof(MsToken);

  out_array->count = count;
  out_array->items = NULL;
  if (count == 0) {
    return 1;
  }

  out_array->items = ms_arena_alloc(parser->arena,
                                    buffer->length,
                                    _Alignof(MsToken));
  if (out_array->items == NULL) {
    return 0;
  }

  memcpy(out_array->items, buffer->data, buffer->length);
  return 1;
}

int ms_parser_copy_map_array(MsParser *parser,
                             const MsBuffer *buffer,
                             MsAstMapEntryArray *out_array) {
  size_t count = buffer->length / sizeof(MsAstMapEntry);

  out_array->count = count;
  out_array->items = NULL;
  if (count == 0) {
    return 1;
  }

  out_array->items = ms_arena_alloc(parser->arena,
                                    buffer->length,
                                    _Alignof(MsAstMapEntry));
  if (out_array->items == NULL) {
    return 0;
  }

  memcpy(out_array->items, buffer->data, buffer->length);
  return 1;
}

int ms_parser_is_assignable(const MsAstNode *node) {
  return node != NULL &&
         (node->kind == MS_AST_VARIABLE ||
          node->kind == MS_AST_PROPERTY ||
          node->kind == MS_AST_INDEX);
}

int ms_parser_infix_precedence(MsTokenKind kind) {
  switch (kind) {
    case MS_TOKEN_EQUAL:
      return MS_PREC_ASSIGNMENT;
    case MS_TOKEN_OR:
      return MS_PREC_LOGICAL_OR;
    case MS_TOKEN_AND:
      return MS_PREC_LOGICAL_AND;
    case MS_TOKEN_EQUAL_EQUAL:
    case MS_TOKEN_BANG_EQUAL:
      return MS_PREC_EQUALITY;
    case MS_TOKEN_LESS:
    case MS_TOKEN_LESS_EQUAL:
    case MS_TOKEN_GREATER:
    case MS_TOKEN_GREATER_EQUAL:
      return MS_PREC_COMPARISON;
    case MS_TOKEN_PLUS:
    case MS_TOKEN_MINUS:
      return MS_PREC_ADDITIVE;
    case MS_TOKEN_STAR:
    case MS_TOKEN_SLASH:
      return MS_PREC_MULTIPLICATIVE;
    case MS_TOKEN_LEFT_PAREN:
    case MS_TOKEN_DOT:
    case MS_TOKEN_LEFT_BRACKET:
      return MS_PREC_POSTFIX;
    default:
      return 0;
  }
}

static MsAstNode *ms_parser_parse_primary(MsParser *parser);
static MsAstNode *ms_parser_parse_call(MsParser *parser, MsAstNode *callee);
static MsAstNode *ms_parser_parse_property(MsParser *parser, MsAstNode *object);
static MsAstNode *ms_parser_parse_index(MsParser *parser, MsAstNode *object);
static MsAstNode *ms_parser_parse_unary(MsParser *parser);
static MsAstNode *ms_parser_parse_list_literal(MsParser *parser,
                                               MsToken left_bracket);
static MsAstNode *ms_parser_parse_tuple_or_grouping(MsParser *parser,
                                                    MsToken left_paren);
static MsAstNode *ms_parser_parse_map_literal(MsParser *parser,
                                              MsToken left_brace);
static MsAstNode *ms_parser_parse_function_expression(MsParser *parser,
                                                      MsToken fn_token);

MsAstNode *ms_parser_make_literal_node(MsParser *parser, MsToken token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_LITERAL,
                                         token.line,
                                         token.column,
                                         token.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.literal.token = token;
  return node;
}

MsAstNode *ms_parser_make_variable_node(MsParser *parser, MsToken name) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_VARIABLE,
                                         name.line,
                                         name.column,
                                         name.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.variable.name = name;
  return node;
}

MsAstNode *ms_parser_make_self_node(MsParser *parser, MsToken keyword) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_SELF,
                                         keyword.line,
                                         keyword.column,
                                         keyword.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.self_expr.keyword = keyword;
  return node;
}

MsAstNode *ms_parser_make_super_node(MsParser *parser,
                                     MsToken keyword,
                                     MsToken method) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_SUPER,
                                         keyword.line,
                                         keyword.column,
                                         method.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.super_expr.keyword = keyword;
  node->as.super_expr.method = method;
  return node;
}

MsAstNode *ms_parser_make_unary_node(MsParser *parser,
                                     MsToken op,
                                     MsAstNode *operand) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_UNARY,
                                         op.line,
                                         op.column,
                                         operand->end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.unary.op = op;
  node->as.unary.operand = operand;
  return node;
}

MsAstNode *ms_parser_make_binary_node(MsParser *parser,
                                      MsAstNode *left,
                                      MsToken op,
                                      MsAstNode *right) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_BINARY,
                                         left->line,
                                         left->column,
                                         right->end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.binary.left = left;
  node->as.binary.op = op;
  node->as.binary.right = right;
  return node;
}

MsAstNode *ms_parser_make_logical_node(MsParser *parser,
                                       MsAstNode *left,
                                       MsToken op,
                                       MsAstNode *right) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_LOGICAL,
                                         left->line,
                                         left->column,
                                         right->end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.logical.left = left;
  node->as.logical.op = op;
  node->as.logical.right = right;
  return node;
}
MsAstNode *ms_parser_make_assign_node(MsParser *parser,
                                      MsAstNode *target,
                                      MsAstNode *value) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_ASSIGN,
                                         target->line,
                                         target->column,
                                         value->end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.assign.target = target;
  node->as.assign.value = value;
  return node;
}

MsAstNode *ms_parser_make_property_node(MsParser *parser,
                                        MsAstNode *object,
                                        MsToken name) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_PROPERTY,
                                         object->line,
                                         object->column,
                                         name.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.property.object = object;
  node->as.property.name = name;
  return node;
}

MsAstNode *ms_parser_make_index_node(MsParser *parser,
                                     MsAstNode *object,
                                     MsAstNode *index_expr,
                                     MsToken close_bracket) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_INDEX,
                                         object->line,
                                         object->column,
                                         close_bracket.end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.index.object = object;
  node->as.index.index = index_expr;
  return node;
}

MsAstNode *ms_parser_make_call_node(MsParser *parser,
                                    MsAstNode *callee,
                                    const MsBuffer *args,
                                    MsToken close_paren) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_CALL,
                                         callee->line,
                                         callee->column,
                                         close_paren.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_node_array(parser, args, &node->as.call.arguments)) {
    ms_parser_error_at_token(parser,
                             close_paren,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.call.callee = callee;
  return node;
}

MsAstNode *ms_parser_make_map_node(MsParser *parser,
                                   const MsBuffer *entries,
                                   int line,
                                   int column,
                                   int end_column) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_MAP,
                                         line,
                                         column,
                                         end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_map_array(parser, entries, &node->as.map.entries)) {
    ms_parser_error_at_token(parser,
                             ms_parser_make_eof_token(parser),
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  return node;
}

MsAstNode *ms_parser_make_sequence_node(MsParser *parser,
                                        MsAstKind kind,
                                        const MsBuffer *elements,
                                        int line,
                                        int column,
                                        int end_column) {
  MsAstNode *node = ms_parser_alloc_node(parser, kind, line, column, end_column);

  if (node == NULL) {
    return NULL;
  }

  if (kind == MS_AST_LIST) {
    if (!ms_parser_copy_node_array(parser, elements, &node->as.list.elements)) {
      ms_parser_error_at_token(parser,
                               ms_parser_make_eof_token(parser),
                               "MS2000",
                               "out of memory while building AST");
      return NULL;
    }
  } else {
    if (!ms_parser_copy_node_array(parser, elements, &node->as.tuple.elements)) {
      ms_parser_error_at_token(parser,
                               ms_parser_make_eof_token(parser),
                               "MS2000",
                               "out of memory while building AST");
      return NULL;
    }
  }

  return node;
}

MsAstNode *ms_parser_make_function_node(MsParser *parser,
                                        MsToken fn_token,
                                        const MsBuffer *params,
                                        MsAstNode *body) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_FUNCTION,
                                         fn_token.line,
                                         fn_token.column,
                                         body->end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_token_array(parser, params, &node->as.function.parameters)) {
    ms_parser_error_at_token(parser,
                             fn_token,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.function.body = body;
  return node;
}

static int ms_parser_parse_parameter_list(MsParser *parser,
                                          MsBuffer *params,
                                          const char *open_message) {
  if (!ms_parser_expect(parser,
                        MS_TOKEN_LEFT_PAREN,
                        "MS2002",
                        open_message,
                        NULL)) {
    return 0;
  }

  ms_parser_skip_newlines(parser);
  if (!ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
    for (;;) {
      MsToken param;

      if (!ms_parser_expect(parser,
                            MS_TOKEN_IDENTIFIER,
                            "MS2002",
                            "expected parameter name",
                            &param) ||
          !ms_buffer_append(params, &param, sizeof(param))) {
        return 0;
      }

      ms_parser_skip_newlines(parser);
      if (ms_parser_match(parser, MS_TOKEN_COMMA)) {
        ms_parser_skip_newlines(parser);
        if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
          break;
        }
        continue;
      }
      if (!ms_parser_expect(parser,
                            MS_TOKEN_RIGHT_PAREN,
                            "MS2002",
                            "expected ')' after parameter list",
                            NULL)) {
        return 0;
      }
      break;
    }
  }

  return 1;
}

static MsAstNode *ms_parser_parse_list_literal(MsParser *parser,
                                               MsToken left_bracket) {
  MsBuffer elements;
  MsAstNode *node;
  MsToken right_bracket;

  ms_buffer_init(&elements);
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_RIGHT_BRACKET)) {
    right_bracket = parser->previous;
    node = ms_parser_make_sequence_node(parser,
                                        MS_AST_LIST,
                                        &elements,
                                        left_bracket.line,
                                        left_bracket.column,
                                        right_bracket.end_column);
    ms_buffer_destroy(&elements);
    return node;
  }

  for (;;) {
    MsAstNode *element;

    ms_parser_skip_newlines(parser);
    element = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
    if (element == NULL ||
        !ms_buffer_append(&elements, &element, sizeof(element))) {
      ms_buffer_destroy(&elements);
      return NULL;
    }

    ms_parser_skip_newlines(parser);
    if (ms_parser_match(parser, MS_TOKEN_COMMA)) {
      ms_parser_skip_newlines(parser);
      if (ms_parser_match(parser, MS_TOKEN_RIGHT_BRACKET)) {
        right_bracket = parser->previous;
        break;
      }
      continue;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_RIGHT_BRACKET,
                          "MS2002",
                          "expected ']' after list literal",
                          &right_bracket)) {
      ms_buffer_destroy(&elements);
      return NULL;
    }
    break;
  }

  node = ms_parser_make_sequence_node(parser,
                                      MS_AST_LIST,
                                      &elements,
                                      left_bracket.line,
                                      left_bracket.column,
                                      right_bracket.end_column);
  ms_buffer_destroy(&elements);
  return node;
}
static MsAstNode *ms_parser_parse_tuple_or_grouping(MsParser *parser,
                                                    MsToken left_paren) {
  MsBuffer elements;
  MsAstNode *first;
  MsAstNode *node;
  MsToken right_paren;

  ms_buffer_init(&elements);
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
    right_paren = parser->previous;
    node = ms_parser_make_sequence_node(parser,
                                        MS_AST_TUPLE,
                                        &elements,
                                        left_paren.line,
                                        left_paren.column,
                                        right_paren.end_column);
    ms_buffer_destroy(&elements);
    return node;
  }

  first = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
  if (first == NULL) {
    ms_buffer_destroy(&elements);
    return NULL;
  }

  ms_parser_skip_newlines(parser);
  if (!ms_parser_match(parser, MS_TOKEN_COMMA)) {
    if (!ms_parser_expect(parser,
                          MS_TOKEN_RIGHT_PAREN,
                          "MS2002",
                          "expected ')' after grouping expression",
                          &right_paren)) {
      ms_buffer_destroy(&elements);
      return NULL;
    }
    ms_buffer_destroy(&elements);
    (void) left_paren;
    (void) right_paren;
    return first;
  }

  if (!ms_buffer_append(&elements, &first, sizeof(first))) {
    ms_buffer_destroy(&elements);
    return NULL;
  }

  ms_parser_skip_newlines(parser);
  if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
    right_paren = parser->previous;
  } else {
    for (;;) {
      MsAstNode *element = ms_parser_parse_expression_prec(parser,
                                                           MS_PREC_ASSIGNMENT);

      if (element == NULL ||
          !ms_buffer_append(&elements, &element, sizeof(element))) {
        ms_buffer_destroy(&elements);
        return NULL;
      }

      ms_parser_skip_newlines(parser);
      if (ms_parser_match(parser, MS_TOKEN_COMMA)) {
        ms_parser_skip_newlines(parser);
        if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
          right_paren = parser->previous;
          break;
        }
        continue;
      }
      if (!ms_parser_expect(parser,
                            MS_TOKEN_RIGHT_PAREN,
                            "MS2002",
                            "expected ')' after tuple literal",
                            &right_paren)) {
        ms_buffer_destroy(&elements);
        return NULL;
      }
      break;
    }
  }

  node = ms_parser_make_sequence_node(parser,
                                      MS_AST_TUPLE,
                                      &elements,
                                      left_paren.line,
                                      left_paren.column,
                                      right_paren.end_column);
  ms_buffer_destroy(&elements);
  return node;
}

static MsAstNode *ms_parser_parse_map_literal(MsParser *parser,
                                              MsToken left_brace) {
  MsBuffer entries;
  MsAstNode *node;
  MsToken right_brace;

  ms_buffer_init(&entries);
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_RIGHT_BRACE)) {
    right_brace = parser->previous;
    node = ms_parser_make_map_node(parser,
                                   &entries,
                                   left_brace.line,
                                   left_brace.column,
                                   right_brace.end_column);
    ms_buffer_destroy(&entries);
    return node;
  }

  for (;;) {
    MsAstNode *key;
    MsAstNode *value;
    MsAstMapEntry entry;

    ms_parser_skip_newlines(parser);
    key = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
    if (key == NULL) {
      ms_buffer_destroy(&entries);
      return NULL;
    }

    ms_parser_skip_newlines(parser);
    if (!ms_parser_expect(parser,
                          MS_TOKEN_COLON,
                          "MS2002",
                          "expected ':' after map key",
                          NULL)) {
      ms_buffer_destroy(&entries);
      return NULL;
    }

    ms_parser_skip_newlines(parser);
    value = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
    if (value == NULL) {
      ms_buffer_destroy(&entries);
      return NULL;
    }

    entry.key = key;
    entry.value = value;
    if (!ms_buffer_append(&entries, &entry, sizeof(entry))) {
      ms_buffer_destroy(&entries);
      return NULL;
    }

    ms_parser_skip_newlines(parser);
    if (ms_parser_match(parser, MS_TOKEN_COMMA)) {
      ms_parser_skip_newlines(parser);
      if (ms_parser_match(parser, MS_TOKEN_RIGHT_BRACE)) {
        right_brace = parser->previous;
        break;
      }
      continue;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_RIGHT_BRACE,
                          "MS2002",
                          "expected '}' after map literal",
                          &right_brace)) {
      ms_buffer_destroy(&entries);
      return NULL;
    }
    break;
  }

  node = ms_parser_make_map_node(parser,
                                 &entries,
                                 left_brace.line,
                                 left_brace.column,
                                 right_brace.end_column);
  ms_buffer_destroy(&entries);
  return node;
}

static MsAstNode *ms_parser_parse_function_expression(MsParser *parser,
                                                      MsToken fn_token) {
  MsBuffer params;
  MsAstNode *body;
  MsAstNode *node;

  ms_buffer_init(&params);
  if (!ms_parser_parse_parameter_list(parser,
                                      &params,
                                      "expected '(' after 'fn'")) {
    ms_buffer_destroy(&params);
    return NULL;
  }

  ms_parser_skip_newlines(parser);
  body = ms_parser_parse_block_statement(parser);
  if (body == NULL) {
    ms_buffer_destroy(&params);
    return NULL;
  }

  node = ms_parser_make_function_node(parser, fn_token, &params, body);
  ms_buffer_destroy(&params);
  return node;
}

static MsAstNode *ms_parser_parse_primary(MsParser *parser) {
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_NUMBER) ||
      ms_parser_match(parser, MS_TOKEN_STRING) ||
      ms_parser_match(parser, MS_TOKEN_TRUE) ||
      ms_parser_match(parser, MS_TOKEN_FALSE) ||
      ms_parser_match(parser, MS_TOKEN_NIL)) {
    return ms_parser_make_literal_node(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_IDENTIFIER)) {
    return ms_parser_make_variable_node(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_SELF)) {
    return ms_parser_make_self_node(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_SUPER)) {
    MsToken keyword = parser->previous;
    MsToken method;

    if (!ms_parser_match(parser, MS_TOKEN_DOT)) {
      ms_parser_error_at_token(parser,
                               keyword,
                               "MS2002",
                               "expected '.' and method name after 'super'");
      return NULL;
    }
    if (!ms_parser_check(parser, MS_TOKEN_IDENTIFIER)) {
      ms_parser_error_at_token(parser,
                               keyword,
                               "MS2002",
                               "expected '.' and method name after 'super'");
      return NULL;
    }
    method = parser->current;
    ms_parser_advance(parser);
    return ms_parser_make_super_node(parser, keyword, method);
  }
  if (ms_parser_match(parser, MS_TOKEN_FN)) {
    return ms_parser_parse_function_expression(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_LEFT_BRACKET)) {
    return ms_parser_parse_list_literal(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_LEFT_BRACE)) {
    return ms_parser_parse_map_literal(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_LEFT_PAREN)) {
    return ms_parser_parse_tuple_or_grouping(parser, parser->previous);
  }

  ms_parser_error_at_token(parser,
                           parser->current,
                           "MS2001",
                           "expected expression");
  return NULL;
}

static MsAstNode *ms_parser_parse_call(MsParser *parser, MsAstNode *callee) {
  MsBuffer arguments;
  MsAstNode *node;
  MsToken close_paren;

  ms_buffer_init(&arguments);
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
    close_paren = parser->previous;
    node = ms_parser_make_call_node(parser, callee, &arguments, close_paren);
    ms_buffer_destroy(&arguments);
    return node;
  }
  for (;;) {
    MsAstNode *argument;

    ms_parser_skip_newlines(parser);
    argument = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
    if (argument == NULL ||
        !ms_buffer_append(&arguments, &argument, sizeof(argument))) {
      ms_buffer_destroy(&arguments);
      return NULL;
    }

    ms_parser_skip_newlines(parser);
    if (ms_parser_match(parser, MS_TOKEN_COMMA)) {
      ms_parser_skip_newlines(parser);
      if (ms_parser_match(parser, MS_TOKEN_RIGHT_PAREN)) {
        close_paren = parser->previous;
        break;
      }
      continue;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_RIGHT_PAREN,
                          "MS2002",
                          "expected ')' after argument list",
                          &close_paren)) {
      ms_buffer_destroy(&arguments);
      return NULL;
    }
    break;
  }

  node = ms_parser_make_call_node(parser, callee, &arguments, close_paren);
  ms_buffer_destroy(&arguments);
  return node;
}

static MsAstNode *ms_parser_parse_property(MsParser *parser, MsAstNode *object) {
  MsToken name;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        "expected property name after '.'",
                        &name)) {
    return NULL;
  }

  return ms_parser_make_property_node(parser, object, name);
}

static MsAstNode *ms_parser_parse_index(MsParser *parser, MsAstNode *object) {
  MsAstNode *index_expr;
  MsToken close_bracket;

  ms_parser_skip_newlines(parser);
  index_expr = ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
  if (index_expr == NULL) {
    return NULL;
  }

  ms_parser_skip_newlines(parser);
  if (!ms_parser_expect(parser,
                        MS_TOKEN_RIGHT_BRACKET,
                        "MS2002",
                        "expected ']' after index expression",
                        &close_bracket)) {
    return NULL;
  }

  return ms_parser_make_index_node(parser, object, index_expr, close_bracket);
}

static MsAstNode *ms_parser_parse_unary(MsParser *parser) {
  if (ms_parser_match(parser, MS_TOKEN_BANG) ||
      ms_parser_match(parser, MS_TOKEN_MINUS)) {
    MsToken op = parser->previous;
    MsAstNode *operand = ms_parser_parse_expression_prec(parser, MS_PREC_UNARY);

    if (operand == NULL) {
      return NULL;
    }
    return ms_parser_make_unary_node(parser, op, operand);
  }

  return ms_parser_parse_primary(parser);
}

void ms_parser_init(MsParser *parser,
                    const char *file,
                    const char *source,
                    MsArena *arena,
                    MsDiagnosticList *diagnostics) {
  parser->file = file != NULL ? file : "<unknown>";
  parser->arena = arena;
  parser->diagnostics = diagnostics;
  parser->next_node_id = 0;
  parser->had_error = 0;
  memset(&parser->previous, 0, sizeof(parser->previous));
  ms_lexer_init(&parser->lexer,
                parser->file,
                source != NULL ? source : "",
                diagnostics);
  parser->current = ms_lexer_next(&parser->lexer);
}

MsAstNode *ms_parser_parse_expression(MsParser *parser) {
  return ms_parser_parse_expression_prec(parser, MS_PREC_ASSIGNMENT);
}

MsAstNode *ms_parser_parse_expression_prec(MsParser *parser, int min_precedence) {
  MsAstNode *left = ms_parser_parse_unary(parser);

  if (left == NULL) {
    return NULL;
  }

  for (;;) {
    MsToken op;
    int precedence;

    op = parser->current;
    precedence = ms_parser_infix_precedence(op.kind);
    if (precedence == 0 || precedence < min_precedence) {
      break;
    }

    ms_parser_advance(parser);
    if (op.kind == MS_TOKEN_LEFT_PAREN) {
      left = ms_parser_parse_call(parser, left);
      if (left == NULL) {
        return NULL;
      }
      continue;
    }
    if (op.kind == MS_TOKEN_DOT) {
      left = ms_parser_parse_property(parser, left);
      if (left == NULL) {
        return NULL;
      }
      continue;
    }
    if (op.kind == MS_TOKEN_LEFT_BRACKET) {
      left = ms_parser_parse_index(parser, left);
      if (left == NULL) {
        return NULL;
      }
      continue;
    }
    if (op.kind == MS_TOKEN_EQUAL) {
      MsAstNode *right = ms_parser_parse_expression_prec(parser, precedence);

      if (right == NULL) {
        return NULL;
      }
      if (!ms_parser_is_assignable(left)) {
        ms_parser_error_at_node(parser,
                                left,
                                "MS2003",
                                "invalid assignment target");
        return NULL;
      }
      left = ms_parser_make_assign_node(parser, left, right);
      if (left == NULL) {
        return NULL;
      }
      continue;
    }

    {
      int next_min = precedence + 1;
      MsAstNode *right = ms_parser_parse_expression_prec(parser, next_min);

      if (right == NULL) {
        return NULL;
      }
      if (op.kind == MS_TOKEN_OR || op.kind == MS_TOKEN_AND) {
        left = ms_parser_make_logical_node(parser, left, op, right);
      } else {
        left = ms_parser_make_binary_node(parser, left, op, right);
      }
      if (left == NULL) {
        return NULL;
      }
    }
  }

  return left;
}

MsAstNode *ms_parse_expression(const char *file,
                               const char *source,
                               MsArena *arena,
                               MsDiagnosticList *diagnostics) {
  MsParser parser;
  MsAstNode *root;

  if (arena == NULL) {
    return NULL;
  }

  ms_parser_init(&parser, file, source, arena, diagnostics);
  ms_parser_skip_newlines(&parser);
  root = ms_parser_parse_expression(&parser);
  if (root == NULL) {
    return NULL;
  }

  ms_parser_skip_newlines(&parser);
  if (!ms_parser_check(&parser, MS_TOKEN_EOF)) {
    ms_parser_error_at_token(&parser,
                             parser.current,
                             "MS2002",
                             "unexpected token after expression");
    return NULL;
  }
  if (parser.had_error) {
    return NULL;
  }

  return root;
}