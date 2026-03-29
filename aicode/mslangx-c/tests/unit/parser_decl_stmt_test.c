#include <stddef.h>
#include <string.h>

#include "ms/arena.h"
#include "ms/ast.h"
#include "ms/diag.h"
#include "ms/parser.h"

#include "test_assert.h"

static int token_equals(MsToken token, const char *expected) {
  size_t length = strlen(expected);

  return token.length == length && memcmp(token.start, expected, length) == 0;
}

static MsAstNode *parse_source(const char *source,
                               MsArena *arena,
                               MsDiagnosticList *diagnostics) {
  ms_arena_init(arena, 4096);
  ms_diag_list_init(diagnostics);
  return ms_parse_source("<unit>", source, arena, diagnostics);
}

static int test_parses_imports_var_function_and_class_declarations(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsAstNode *root = parse_source(
      "import core.io\n"
      "from tools.util import helper as alias\n"
      "var answer\n"
      "fn compute(limit) {\n"
      "  return answer\n"
      "}\n"
      "class Derived < Base {\n"
      "  run() {\n"
      "    return super.run\n"
      "  }\n"
      "}\n",
      &arena,
      &diagnostics);

  TEST_ASSERT(root != NULL);
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(root->kind == MS_AST_PROGRAM);
  TEST_ASSERT(root->as.program.declarations.count == 5);

  TEST_ASSERT(root->as.program.declarations.items[0]->kind == MS_AST_IMPORT_STMT);
  TEST_ASSERT(root->as.program.declarations.items[0]->as.import_stmt.path.count == 2);
  TEST_ASSERT(token_equals(root->as.program.declarations.items[0]->as.import_stmt.path.items[0],
                           "core"));
  TEST_ASSERT(token_equals(root->as.program.declarations.items[0]->as.import_stmt.path.items[1],
                           "io"));

  TEST_ASSERT(root->as.program.declarations.items[1]->kind == MS_AST_FROM_IMPORT_STMT);
  TEST_ASSERT(root->as.program.declarations.items[1]->as.from_import_stmt.path.count == 2);
  TEST_ASSERT(token_equals(root->as.program.declarations.items[1]->as.from_import_stmt.name,
                           "helper"));
  TEST_ASSERT(token_equals(root->as.program.declarations.items[1]->as.from_import_stmt.alias,
                           "alias"));

  TEST_ASSERT(root->as.program.declarations.items[2]->kind == MS_AST_VAR_DECL);
  TEST_ASSERT(token_equals(root->as.program.declarations.items[2]->as.var_decl.name,
                           "answer"));
  TEST_ASSERT(root->as.program.declarations.items[2]->as.var_decl.initializer != NULL);
  TEST_ASSERT(root->as.program.declarations.items[2]->as.var_decl.initializer->kind ==
              MS_AST_LITERAL);
  TEST_ASSERT(root->as.program.declarations.items[2]->as.var_decl.initializer->as.literal.token.kind ==
              MS_TOKEN_NIL);

  TEST_ASSERT(root->as.program.declarations.items[3]->kind == MS_AST_FUNCTION_DECL);
  TEST_ASSERT(token_equals(root->as.program.declarations.items[3]->as.function_decl.name,
                           "compute"));
  TEST_ASSERT(root->as.program.declarations.items[3]->as.function_decl.parameters.count == 1);
  TEST_ASSERT(root->as.program.declarations.items[3]->as.function_decl.body->kind ==
              MS_AST_BLOCK);

  TEST_ASSERT(root->as.program.declarations.items[4]->kind == MS_AST_CLASS_DECL);
  TEST_ASSERT(token_equals(root->as.program.declarations.items[4]->as.class_decl.name,
                           "Derived"));
  TEST_ASSERT(token_equals(root->as.program.declarations.items[4]->as.class_decl.superclass,
                           "Base"));
  TEST_ASSERT(root->as.program.declarations.items[4]->as.class_decl.methods.count == 1);
  TEST_ASSERT(root->as.program.declarations.items[4]->as.class_decl.methods.items[0]->kind ==
              MS_AST_FUNCTION_DECL);

  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_parses_control_flow_statements_and_terminators(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsAstNode *root = parse_source(
      "fn control(limit) {\n"
      "  if (limit) print limit\n"
      "  else print nil\n"
      "  while (limit) {\n"
      "    print limit\n"
      "    break\n"
      "  }\n"
      "  for (var i = 0; i < limit; i = i + 1) {\n"
      "    continue\n"
      "  }\n"
      "  {\n"
      "    print limit\n"
      "  }\n"
      "  control(limit)\n"
      "  return limit\n"
      "}\n",
      &arena,
      &diagnostics);
  MsAstNode *body;

  TEST_ASSERT(root != NULL);
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(root->kind == MS_AST_PROGRAM);
  TEST_ASSERT(root->as.program.declarations.count == 1);
  TEST_ASSERT(root->as.program.declarations.items[0]->kind == MS_AST_FUNCTION_DECL);

  body = root->as.program.declarations.items[0]->as.function_decl.body;
  TEST_ASSERT(body != NULL);
  TEST_ASSERT(body->kind == MS_AST_BLOCK);
  TEST_ASSERT(body->as.block.statements.count == 6);
  TEST_ASSERT(body->as.block.statements.items[0]->kind == MS_AST_IF_STMT);
  TEST_ASSERT(body->as.block.statements.items[1]->kind == MS_AST_WHILE_STMT);
  TEST_ASSERT(body->as.block.statements.items[2]->kind == MS_AST_FOR_STMT);
  TEST_ASSERT(body->as.block.statements.items[3]->kind == MS_AST_BLOCK);
  TEST_ASSERT(body->as.block.statements.items[4]->kind == MS_AST_EXPR_STMT);
  TEST_ASSERT(body->as.block.statements.items[5]->kind == MS_AST_RETURN_STMT);
  TEST_ASSERT(body->as.block.statements.items[2]->as.for_stmt.initializer->kind ==
              MS_AST_VAR_DECL);

  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_requires_literal_for_header_semicolons_and_recovers(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsAstNode *root = parse_source(
      "print 1\n"
      "for (var i = 0\n"
      "     i < 3; i = i + 1) print i\n"
      "print 2\n",
      &arena,
      &diagnostics);
  const MsDiagnostic *diagnostic = NULL;
  size_t i;
  int found = 0;

  TEST_ASSERT(root != NULL);
  TEST_ASSERT(root->kind == MS_AST_PROGRAM);
  TEST_ASSERT(root->as.program.declarations.count >= 2);
  TEST_ASSERT(ms_diag_list_count(&diagnostics) >= 1);
  TEST_ASSERT(root->as.program.declarations.items[0]->kind == MS_AST_PRINT_STMT);
  TEST_ASSERT(root->as.program.declarations.items[root->as.program.declarations.count - 1]->kind ==
              MS_AST_PRINT_STMT);

  for (i = 0; i < ms_diag_list_count(&diagnostics); ++i) {
    diagnostic = ms_diag_list_at(&diagnostics, i);
    if (diagnostic != NULL &&
        strcmp(diagnostic->phase, "parse") == 0 &&
        strcmp(diagnostic->code, "MS2002") == 0 &&
        strcmp(diagnostic->message,
               "expected ';' after for-loop initializer") == 0) {
      found = 1;
      TEST_ASSERT(diagnostic->span.line == 2);
      TEST_ASSERT(diagnostic->span.column == 15);
      break;
    }
  }
  TEST_ASSERT(found);

  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_recovers_multiple_errors_and_keeps_later_statements(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsAstNode *root = parse_source(
      "var first =\n"
      "print 1\n"
      "from pkg.tools import as\n"
      "print 2\n",
      &arena,
      &diagnostics);
  const MsDiagnostic *diagnostic;

  TEST_ASSERT(root != NULL);
  TEST_ASSERT(root->kind == MS_AST_PROGRAM);
  TEST_ASSERT(root->as.program.declarations.count >= 2);
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 2);

  diagnostic = ms_diag_list_at(&diagnostics, 0);
  TEST_ASSERT(diagnostic != NULL);
  TEST_ASSERT(strcmp(diagnostic->code, "MS2001") == 0);
  TEST_ASSERT(strcmp(diagnostic->message, "expected expression") == 0);
  TEST_ASSERT(diagnostic->span.line == 2);
  TEST_ASSERT(diagnostic->span.column == 1);

  diagnostic = ms_diag_list_at(&diagnostics, 1);
  TEST_ASSERT(diagnostic != NULL);
  TEST_ASSERT(strcmp(diagnostic->code, "MS2002") == 0);
  TEST_ASSERT(strcmp(diagnostic->message,
                     "expected imported name after 'import'") == 0);
  TEST_ASSERT(diagnostic->span.line == 3);
  TEST_ASSERT(diagnostic->span.column == 23);

  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

int main(void) {
  TEST_ASSERT(test_parses_imports_var_function_and_class_declarations() == 0);
  TEST_ASSERT(test_parses_control_flow_statements_and_terminators() == 0);
  TEST_ASSERT(test_requires_literal_for_header_semicolons_and_recovers() == 0);
  TEST_ASSERT(test_recovers_multiple_errors_and_keeps_later_statements() == 0);
  return 0;
}