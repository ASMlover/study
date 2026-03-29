#include "ms/parser.h"

#include <string.h>

#include "ms/buffer.h"

#include "parser_internal.h"

static MsAstNode *ms_parser_make_program_node(MsParser *parser,
                                              const MsBuffer *declarations,
                                              MsToken start,
                                              MsToken end) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_PROGRAM,
                                         start.line,
                                         start.column,
                                         end.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_node_array(parser,
                                 declarations,
                                 &node->as.program.declarations)) {
    ms_parser_error_at_token(parser,
                             end,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }
  return node;
}

static MsAstNode *ms_parser_make_block_node(MsParser *parser,
                                            MsToken left_brace,
                                            MsToken right_brace,
                                            const MsBuffer *statements) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_BLOCK,
                                         left_brace.line,
                                         left_brace.column,
                                         right_brace.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_node_array(parser,
                                 statements,
                                 &node->as.block.statements)) {
    ms_parser_error_at_token(parser,
                             right_brace,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }
  return node;
}

static MsAstNode *ms_parser_make_var_decl_node(MsParser *parser,
                                               MsToken keyword,
                                               MsToken name,
                                               MsAstNode *initializer) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_VAR_DECL,
                                         keyword.line,
                                         keyword.column,
                                         initializer->end_column);

  if (node == NULL) {
    return NULL;
  }

  node->as.var_decl.name = name;
  node->as.var_decl.initializer = initializer;
  return node;
}

static MsAstNode *ms_parser_make_function_decl_node(MsParser *parser,
                                                    MsToken start,
                                                    MsToken name,
                                                    const MsBuffer *params,
                                                    MsAstNode *body) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_FUNCTION_DECL,
                                         start.line,
                                         start.column,
                                         body->end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_token_array(parser,
                                  params,
                                  &node->as.function_decl.parameters)) {
    ms_parser_error_at_token(parser,
                             name,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.function_decl.name = name;
  node->as.function_decl.body = body;
  return node;
}

static MsAstNode *ms_parser_make_class_decl_node(MsParser *parser,
                                                 MsToken class_token,
                                                 MsToken name,
                                                 MsToken superclass,
                                                 const MsBuffer *methods,
                                                 MsToken right_brace) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_CLASS_DECL,
                                         class_token.line,
                                         class_token.column,
                                         right_brace.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_node_array(parser,
                                 methods,
                                 &node->as.class_decl.methods)) {
    ms_parser_error_at_token(parser,
                             right_brace,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.class_decl.name = name;
  node->as.class_decl.superclass = superclass;
  return node;
}

static MsAstNode *ms_parser_make_import_stmt_node(MsParser *parser,
                                                  MsToken keyword,
                                                  const MsBuffer *path,
                                                  MsToken alias,
                                                  MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_IMPORT_STMT,
                                         keyword.line,
                                         keyword.column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_token_array(parser, path, &node->as.import_stmt.path)) {
    ms_parser_error_at_token(parser,
                             end_token,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.import_stmt.alias = alias;
  return node;
}

static MsAstNode *ms_parser_make_from_import_stmt_node(MsParser *parser,
                                                       MsToken from_token,
                                                       const MsBuffer *path,
                                                       MsToken name,
                                                       MsToken alias,
                                                       MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_FROM_IMPORT_STMT,
                                         from_token.line,
                                         from_token.column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (!ms_parser_copy_token_array(parser,
                                  path,
                                  &node->as.from_import_stmt.path)) {
    ms_parser_error_at_token(parser,
                             end_token,
                             "MS2000",
                             "out of memory while building AST");
    return NULL;
  }

  node->as.from_import_stmt.name = name;
  node->as.from_import_stmt.alias = alias;
  return node;
}

static MsAstNode *ms_parser_make_expr_stmt_node(MsParser *parser,
                                                MsAstNode *expression,
                                                MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_EXPR_STMT,
                                         expression->line,
                                         expression->column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.expr_stmt.expression = expression;
  return node;
}

static MsAstNode *ms_parser_make_print_stmt_node(MsParser *parser,
                                                 MsToken keyword,
                                                 MsAstNode *expression,
                                                 MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_PRINT_STMT,
                                         keyword.line,
                                         keyword.column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.print_stmt.expression = expression;
  return node;
}

static MsAstNode *ms_parser_make_return_stmt_node(MsParser *parser,
                                                  MsToken keyword,
                                                  MsAstNode *value,
                                                  MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_RETURN_STMT,
                                         keyword.line,
                                         keyword.column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.return_stmt.keyword = keyword;
  node->as.return_stmt.value = value;
  return node;
}

static MsAstNode *ms_parser_make_keyword_stmt_node(MsParser *parser,
                                                   MsAstKind kind,
                                                   MsToken keyword,
                                                   MsToken end_token) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         kind,
                                         keyword.line,
                                         keyword.column,
                                         end_token.end_column);

  if (node == NULL) {
    return NULL;
  }
  if (kind == MS_AST_BREAK_STMT) {
    node->as.break_stmt.keyword = keyword;
  } else {
    node->as.continue_stmt.keyword = keyword;
  }
  return node;
}

static MsAstNode *ms_parser_make_if_stmt_node(MsParser *parser,
                                              MsToken keyword,
                                              MsAstNode *condition,
                                              MsAstNode *then_branch,
                                              MsAstNode *else_branch) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_IF_STMT,
                                         keyword.line,
                                         keyword.column,
                                         else_branch != NULL ? else_branch->end_column
                                                             : then_branch->end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.if_stmt.condition = condition;
  node->as.if_stmt.then_branch = then_branch;
  node->as.if_stmt.else_branch = else_branch;
  return node;
}

static MsAstNode *ms_parser_make_while_stmt_node(MsParser *parser,
                                                 MsToken keyword,
                                                 MsAstNode *condition,
                                                 MsAstNode *body) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_WHILE_STMT,
                                         keyword.line,
                                         keyword.column,
                                         body->end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.while_stmt.condition = condition;
  node->as.while_stmt.body = body;
  return node;
}

static MsAstNode *ms_parser_make_for_stmt_node(MsParser *parser,
                                               MsToken keyword,
                                               MsAstNode *initializer,
                                               MsAstNode *condition,
                                               MsAstNode *increment,
                                               MsAstNode *body) {
  MsAstNode *node = ms_parser_alloc_node(parser,
                                         MS_AST_FOR_STMT,
                                         keyword.line,
                                         keyword.column,
                                         body->end_column);

  if (node == NULL) {
    return NULL;
  }
  node->as.for_stmt.initializer = initializer;
  node->as.for_stmt.condition = condition;
  node->as.for_stmt.increment = increment;
  node->as.for_stmt.body = body;
  return node;
}

static int ms_parser_is_implicit_terminator(const MsParser *parser) {
  return ms_parser_check(parser, MS_TOKEN_RIGHT_BRACE) ||
         ms_parser_check(parser, MS_TOKEN_EOF);
}

static MsToken ms_parser_current_or_previous_terminator(MsParser *parser) {
  if (parser->previous.kind == MS_TOKEN_SEMICOLON ||
      parser->previous.kind == MS_TOKEN_NEWLINE) {
    return parser->previous;
  }
  return parser->current;
}

int ms_parser_consume_terminator(MsParser *parser, const char *message) {
  if (ms_parser_match(parser, MS_TOKEN_SEMICOLON)) {
    ms_parser_skip_newlines(parser);
    return 1;
  }
  if (ms_parser_match(parser, MS_TOKEN_NEWLINE)) {
    ms_parser_skip_newlines(parser);
    return 1;
  }
  if (ms_parser_is_implicit_terminator(parser)) {
    return 1;
  }

  ms_parser_error_at_token(parser, parser->current, "MS2002", message);
  return 0;
}

static int ms_parser_parse_module_path(MsParser *parser,
                                       MsBuffer *segments,
                                       const char *message) {
  MsToken segment;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        message,
                        &segment) ||
      !ms_buffer_append(segments, &segment, sizeof(segment))) {
    return 0;
  }

  while (ms_parser_match(parser, MS_TOKEN_DOT)) {
    if (!ms_parser_expect(parser,
                          MS_TOKEN_IDENTIFIER,
                          "MS2002",
                          "expected identifier after '.' in module path",
                          &segment) ||
        !ms_buffer_append(segments, &segment, sizeof(segment))) {
      return 0;
    }
  }

  return 1;
}

static MsToken ms_parser_make_implicit_nil_token(MsToken name) {
  MsToken token;

  token.kind = MS_TOKEN_NIL;
  token.start = "nil";
  token.length = 3;
  token.line = name.line;
  token.column = name.column;
  token.end_column = name.column;
  return token;
}

static MsAstNode *ms_parser_parse_statement(MsParser *parser);
static MsAstNode *ms_parser_parse_var_decl_after_keyword(MsParser *parser,
                                                         MsToken keyword,
                                                         int consume_terminator);
static MsAstNode *ms_parser_parse_named_function_decl(MsParser *parser,
                                                      MsToken start,
                                                      MsToken name);
static MsAstNode *ms_parser_parse_var_decl_after_keyword(MsParser *parser,
                                                         MsToken keyword,
                                                         int consume_terminator) {
  MsToken name;
  MsAstNode *initializer;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        "expected variable name",
                        &name)) {
    return NULL;
  }

  if (ms_parser_match(parser, MS_TOKEN_EQUAL)) {
    initializer = ms_parser_parse_expression(parser);
    if (initializer == NULL) {
      return NULL;
    }
  } else {
    initializer = ms_parser_make_literal_node(parser,
                                              ms_parser_make_implicit_nil_token(name));
    if (initializer == NULL) {
      return NULL;
    }
  }

  if (consume_terminator &&
      !ms_parser_consume_terminator(parser,
                                    "expected statement terminator after variable declaration")) {
    return NULL;
  }

  return ms_parser_make_var_decl_node(parser, keyword, name, initializer);
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

static MsAstNode *ms_parser_parse_named_function_decl(MsParser *parser,
                                                      MsToken start,
                                                      MsToken name) {
  MsBuffer params;
  MsAstNode *body;
  MsAstNode *node;

  ms_buffer_init(&params);
  if (!ms_parser_parse_parameter_list(parser,
                                      &params,
                                      "expected '(' before parameter list")) {
    ms_buffer_destroy(&params);
    return NULL;
  }

  ms_parser_skip_newlines(parser);
  body = ms_parser_parse_block_statement(parser);
  if (body == NULL) {
    ms_buffer_destroy(&params);
    return NULL;
  }

  node = ms_parser_make_function_decl_node(parser, start, name, &params, body);
  ms_buffer_destroy(&params);
  return node;
}

static MsAstNode *ms_parser_parse_if_stmt(MsParser *parser, MsToken keyword) {
  MsAstNode *condition;
  MsAstNode *then_branch;
  MsAstNode *else_branch = NULL;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_LEFT_PAREN,
                        "MS2002",
                        "expected '(' after 'if'",
                        NULL)) {
    return NULL;
  }

  condition = ms_parser_parse_expression(parser);
  if (condition == NULL) {
    return NULL;
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_RIGHT_PAREN,
                        "MS2002",
                        "expected ')' after if condition",
                        NULL)) {
    return NULL;
  }

  then_branch = ms_parser_parse_statement(parser);
  if (then_branch == NULL) {
    return NULL;
  }

  if (ms_parser_match(parser, MS_TOKEN_ELSE)) {
    else_branch = ms_parser_parse_statement(parser);
    if (else_branch == NULL) {
      return NULL;
    }
  }

  return ms_parser_make_if_stmt_node(parser,
                                     keyword,
                                     condition,
                                     then_branch,
                                     else_branch);
}

static MsAstNode *ms_parser_parse_while_stmt(MsParser *parser, MsToken keyword) {
  MsAstNode *condition;
  MsAstNode *body;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_LEFT_PAREN,
                        "MS2002",
                        "expected '(' after 'while'",
                        NULL)) {
    return NULL;
  }

  condition = ms_parser_parse_expression(parser);
  if (condition == NULL) {
    return NULL;
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_RIGHT_PAREN,
                        "MS2002",
                        "expected ')' after while condition",
                        NULL)) {
    return NULL;
  }

  body = ms_parser_parse_statement(parser);
  if (body == NULL) {
    return NULL;
  }

  return ms_parser_make_while_stmt_node(parser, keyword, condition, body);
}

static MsAstNode *ms_parser_parse_for_stmt(MsParser *parser, MsToken keyword) {
  MsAstNode *initializer = NULL;
  MsAstNode *condition = NULL;
  MsAstNode *increment = NULL;
  MsAstNode *body;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_LEFT_PAREN,
                        "MS2002",
                        "expected '(' after 'for'",
                        NULL)) {
    return NULL;
  }

  if (ms_parser_match(parser, MS_TOKEN_SEMICOLON)) {
  } else if (ms_parser_match(parser, MS_TOKEN_VAR)) {
    initializer = ms_parser_parse_var_decl_after_keyword(parser,
                                                         parser->previous,
                                                         0);
    if (initializer == NULL) {
      return NULL;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_SEMICOLON,
                          "MS2002",
                          "expected ';' after for-loop initializer",
                          NULL)) {
      return NULL;
    }
  } else {
    initializer = ms_parser_parse_expression(parser);
    if (initializer == NULL) {
      return NULL;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_SEMICOLON,
                          "MS2002",
                          "expected ';' after for-loop initializer",
                          NULL)) {
      return NULL;
    }
  }

  if (!ms_parser_check(parser, MS_TOKEN_SEMICOLON)) {
    condition = ms_parser_parse_expression(parser);
    if (condition == NULL) {
      return NULL;
    }
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_SEMICOLON,
                        "MS2002",
                        "expected ';' after for-loop condition",
                        NULL)) {
    return NULL;
  }

  if (!ms_parser_check(parser, MS_TOKEN_RIGHT_PAREN)) {
    increment = ms_parser_parse_expression(parser);
    if (increment == NULL) {
      return NULL;
    }
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_RIGHT_PAREN,
                        "MS2002",
                        "expected ')' after for clauses",
                        NULL)) {
    return NULL;
  }

  body = ms_parser_parse_statement(parser);
  if (body == NULL) {
    return NULL;
  }

  return ms_parser_make_for_stmt_node(parser,
                                      keyword,
                                      initializer,
                                      condition,
                                      increment,
                                      body);
}

static MsAstNode *ms_parser_parse_return_stmt(MsParser *parser, MsToken keyword) {
  MsAstNode *value = NULL;
  MsToken end_token;

  if (!ms_parser_check(parser, MS_TOKEN_SEMICOLON) &&
      !ms_parser_check(parser, MS_TOKEN_NEWLINE) &&
      !ms_parser_is_implicit_terminator(parser)) {
    value = ms_parser_parse_expression(parser);
    if (value == NULL) {
      return NULL;
    }
  }

  if (!ms_parser_consume_terminator(parser,
                                    "expected statement terminator after return statement")) {
    return NULL;
  }
  end_token = ms_parser_current_or_previous_terminator(parser);
  return ms_parser_make_return_stmt_node(parser, keyword, value, end_token);
}

static MsAstNode *ms_parser_parse_keyword_stmt(MsParser *parser,
                                               MsAstKind kind,
                                               MsToken keyword,
                                               const char *message) {
  MsToken end_token;

  if (!ms_parser_consume_terminator(parser, message)) {
    return NULL;
  }
  end_token = ms_parser_current_or_previous_terminator(parser);
  return ms_parser_make_keyword_stmt_node(parser, kind, keyword, end_token);
}

static MsAstNode *ms_parser_parse_print_stmt(MsParser *parser, MsToken keyword) {
  MsAstNode *expression = ms_parser_parse_expression(parser);
  MsToken end_token;

  if (expression == NULL) {
    return NULL;
  }
  if (!ms_parser_consume_terminator(parser,
                                    "expected statement terminator after print statement")) {
    return NULL;
  }
  end_token = ms_parser_current_or_previous_terminator(parser);
  return ms_parser_make_print_stmt_node(parser, keyword, expression, end_token);
}

static MsAstNode *ms_parser_parse_import_stmt(MsParser *parser, MsToken keyword) {
  MsBuffer path;
  MsToken alias = {0};
  MsToken end_token;
  MsAstNode *node;

  ms_buffer_init(&path);
  if (!ms_parser_parse_module_path(parser,
                                   &path,
                                   "expected module path after 'import'")) {
    ms_buffer_destroy(&path);
    return NULL;
  }

  if (ms_parser_match(parser, MS_TOKEN_AS) &&
      !ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        "expected alias after 'as'",
                        &alias)) {
    ms_buffer_destroy(&path);
    return NULL;
  }

  if (!ms_parser_consume_terminator(parser,
                                    "expected statement terminator after import statement")) {
    ms_buffer_destroy(&path);
    return NULL;
  }

  end_token = ms_parser_current_or_previous_terminator(parser);
  node = ms_parser_make_import_stmt_node(parser, keyword, &path, alias, end_token);
  ms_buffer_destroy(&path);
  return node;
}
static MsAstNode *ms_parser_parse_from_import_stmt(MsParser *parser,
                                                   MsToken from_token) {
  MsBuffer path;
  MsToken name;
  MsToken alias = {0};
  MsToken end_token;
  MsAstNode *node;

  ms_buffer_init(&path);
  if (!ms_parser_parse_module_path(parser,
                                   &path,
                                   "expected module path after 'from'")) {
    ms_buffer_destroy(&path);
    return NULL;
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_IMPORT,
                        "MS2002",
                        "expected 'import' after module path",
                        NULL)) {
    ms_buffer_destroy(&path);
    return NULL;
  }
  if (!ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        "expected imported name after 'import'",
                        &name)) {
    ms_buffer_destroy(&path);
    return NULL;
  }
  if (ms_parser_match(parser, MS_TOKEN_AS) &&
      !ms_parser_expect(parser,
                        MS_TOKEN_IDENTIFIER,
                        "MS2002",
                        "expected alias after 'as'",
                        &alias)) {
    ms_buffer_destroy(&path);
    return NULL;
  }
  if (!ms_parser_consume_terminator(parser,
                                    "expected statement terminator after from-import statement")) {
    ms_buffer_destroy(&path);
    return NULL;
  }

  end_token = ms_parser_current_or_previous_terminator(parser);
  node = ms_parser_make_from_import_stmt_node(parser,
                                              from_token,
                                              &path,
                                              name,
                                              alias,
                                              end_token);
  ms_buffer_destroy(&path);
  return node;
}

static MsAstNode *ms_parser_parse_expr_stmt(MsParser *parser) {
  MsAstNode *expression = ms_parser_parse_expression(parser);
  MsToken end_token;

  if (expression == NULL) {
    return NULL;
  }
  if (!ms_parser_consume_terminator(parser,
                                    "expected statement terminator after expression")) {
    return NULL;
  }
  end_token = ms_parser_current_or_previous_terminator(parser);
  return ms_parser_make_expr_stmt_node(parser, expression, end_token);
}

static MsAstNode *ms_parser_parse_statement(MsParser *parser) {
  ms_parser_skip_newlines(parser);

  if (ms_parser_check(parser, MS_TOKEN_LEFT_BRACE)) {
    return ms_parser_parse_block_statement(parser);
  }
  if (ms_parser_match(parser, MS_TOKEN_IF)) {
    return ms_parser_parse_if_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_WHILE)) {
    return ms_parser_parse_while_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_FOR)) {
    return ms_parser_parse_for_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_RETURN)) {
    return ms_parser_parse_return_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_BREAK)) {
    return ms_parser_parse_keyword_stmt(parser,
                                        MS_AST_BREAK_STMT,
                                        parser->previous,
                                        "expected statement terminator after break statement");
  }
  if (ms_parser_match(parser, MS_TOKEN_CONTINUE)) {
    return ms_parser_parse_keyword_stmt(parser,
                                        MS_AST_CONTINUE_STMT,
                                        parser->previous,
                                        "expected statement terminator after continue statement");
  }
  if (ms_parser_match(parser, MS_TOKEN_PRINT)) {
    return ms_parser_parse_print_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_IMPORT)) {
    return ms_parser_parse_import_stmt(parser, parser->previous);
  }
  if (ms_parser_match(parser, MS_TOKEN_FROM)) {
    return ms_parser_parse_from_import_stmt(parser, parser->previous);
  }

  return ms_parser_parse_expr_stmt(parser);
}

MsAstNode *ms_parser_parse_declaration(MsParser *parser) {
  ms_parser_skip_newlines(parser);

  if (ms_parser_match(parser, MS_TOKEN_CLASS)) {
    MsToken class_token = parser->previous;
    MsToken name;
    MsToken superclass = {0};
    MsBuffer methods;
    MsToken right_brace;
    MsAstNode *node;

    if (!ms_parser_expect(parser,
                          MS_TOKEN_IDENTIFIER,
                          "MS2002",
                          "expected class name",
                          &name)) {
      return NULL;
    }
    if (ms_parser_match(parser, MS_TOKEN_LESS) &&
        !ms_parser_expect(parser,
                          MS_TOKEN_IDENTIFIER,
                          "MS2002",
                          "expected superclass name after '<'",
                          &superclass)) {
      return NULL;
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_LEFT_BRACE,
                          "MS2002",
                          "expected '{' before class body",
                          NULL)) {
      return NULL;
    }

    ms_buffer_init(&methods);
    ms_parser_skip_newlines(parser);
    while (!ms_parser_check(parser, MS_TOKEN_RIGHT_BRACE) &&
           !ms_parser_check(parser, MS_TOKEN_EOF)) {
      MsToken method_name;
      MsAstNode *method;

      if (!ms_parser_expect(parser,
                            MS_TOKEN_IDENTIFIER,
                            "MS2002",
                            "expected method name",
                            &method_name)) {
        ms_buffer_destroy(&methods);
        return NULL;
      }
      method = ms_parser_parse_named_function_decl(parser, method_name, method_name);
      if (method == NULL ||
          !ms_buffer_append(&methods, &method, sizeof(method))) {
        ms_buffer_destroy(&methods);
        return NULL;
      }
      ms_parser_skip_newlines(parser);
    }
    if (!ms_parser_expect(parser,
                          MS_TOKEN_RIGHT_BRACE,
                          "MS2002",
                          "expected '}' after class body",
                          &right_brace)) {
      ms_buffer_destroy(&methods);
      return NULL;
    }

    node = ms_parser_make_class_decl_node(parser,
                                          class_token,
                                          name,
                                          superclass,
                                          &methods,
                                          right_brace);
    ms_buffer_destroy(&methods);
    return node;
  }

  if (ms_parser_match(parser, MS_TOKEN_FN)) {
    MsToken fn_token = parser->previous;
    MsToken name;

    if (!ms_parser_expect(parser,
                          MS_TOKEN_IDENTIFIER,
                          "MS2002",
                          "expected function name",
                          &name)) {
      return NULL;
    }
    return ms_parser_parse_named_function_decl(parser, fn_token, name);
  }

  if (ms_parser_match(parser, MS_TOKEN_VAR)) {
    return ms_parser_parse_var_decl_after_keyword(parser, parser->previous, 1);
  }

  return ms_parser_parse_statement(parser);
}

MsAstNode *ms_parser_parse_block_statement(MsParser *parser) {
  MsToken left_brace;
  MsToken right_brace;
  MsBuffer statements;
  MsAstNode *node;

  if (!ms_parser_expect(parser,
                        MS_TOKEN_LEFT_BRACE,
                        "MS2002",
                        "expected '{' before block",
                        &left_brace)) {
    return NULL;
  }

  ms_buffer_init(&statements);
  ms_parser_skip_newlines(parser);
  while (!ms_parser_check(parser, MS_TOKEN_RIGHT_BRACE) &&
         !ms_parser_check(parser, MS_TOKEN_EOF)) {
    MsAstNode *statement = ms_parser_parse_declaration(parser);

    if (statement == NULL ||
        !ms_buffer_append(&statements, &statement, sizeof(statement))) {
      ms_buffer_destroy(&statements);
      return NULL;
    }
    ms_parser_skip_newlines(parser);
  }

  if (!ms_parser_expect(parser,
                        MS_TOKEN_RIGHT_BRACE,
                        "MS2002",
                        "expected '}' after block",
                        &right_brace)) {
    ms_buffer_destroy(&statements);
    return NULL;
  }

  node = ms_parser_make_block_node(parser, left_brace, right_brace, &statements);
  ms_buffer_destroy(&statements);
  return node;
}

MsAstNode *ms_parse_source(const char *file,
                           const char *source,
                           MsArena *arena,
                           MsDiagnosticList *diagnostics) {
  MsParser parser;
  MsBuffer declarations;
  MsAstNode *root;
  MsToken start;
  MsToken end;

  if (arena == NULL) {
    return NULL;
  }

  ms_parser_init(&parser, file, source, arena, diagnostics);
  ms_buffer_init(&declarations);
  ms_parser_skip_newlines(&parser);
  start = parser.current;

  while (!ms_parser_check(&parser, MS_TOKEN_EOF)) {
    MsAstNode *declaration = ms_parser_parse_declaration(&parser);

    if (declaration != NULL) {
      if (!ms_buffer_append(&declarations, &declaration, sizeof(declaration))) {
        ms_buffer_destroy(&declarations);
        return NULL;
      }
    } else {
      ms_parser_synchronize(&parser);
    }
    ms_parser_skip_newlines(&parser);
  }

  end = parser.current;
  root = ms_parser_make_program_node(&parser, &declarations, start, end);
  ms_buffer_destroy(&declarations);
  return root;
}