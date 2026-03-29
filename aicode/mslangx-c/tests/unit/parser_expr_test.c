#include <stddef.h>
#include <string.h>

#include "ms/arena.h"
#include "ms/ast.h"
#include "ms/buffer.h"
#include "ms/diag.h"
#include "ms/parser.h"

#include "test_assert.h"

static int parse_expression_snapshot(const char *source,
                                     MsArena *arena,
                                     MsDiagnosticList *diagnostics,
                                     MsBuffer *snapshot,
                                     MsAstNode **out_root) {
  MsAstNode *root;

  ms_arena_init(arena, 1024);
  ms_diag_list_init(diagnostics);
  ms_buffer_init(snapshot);

  root = ms_parse_expression("<unit>", source, arena, diagnostics);
  if (out_root != NULL) {
    *out_root = root;
  }
  if (root == NULL) {
    return 0;
  }

  return ms_ast_dump(snapshot, root);
}

static int test_assignment_precedence_and_spans(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsBuffer snapshot;
  MsAstNode *root = NULL;
  const char *expected =
      "assign\n"
      "  variable \"a\"\n"
      "  assign\n"
      "    variable \"b\"\n"
      "    logical \"or\"\n"
      "      variable \"c\"\n"
      "      logical \"and\"\n"
      "        variable \"d\"\n"
      "        binary \"==\"\n"
      "          variable \"e\"\n"
      "          binary \"<\"\n"
      "            variable \"f\"\n"
      "            binary \"+\"\n"
      "              variable \"g\"\n"
      "              binary \"*\"\n"
      "                variable \"h\"\n"
      "                unary \"-\"\n"
      "                  variable \"i\"\n";

  TEST_ASSERT(parse_expression_snapshot("a = b = c or d and e == f < g + h * -i\n",
                                        &arena,
                                        &diagnostics,
                                        &snapshot,
                                        &root));
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(root != NULL);
  TEST_ASSERT(root->kind == MS_AST_ASSIGN);
  TEST_ASSERT(root->node_id > 0);
  TEST_ASSERT(root->line == 1);
  TEST_ASSERT(root->column == 1);
  TEST_ASSERT(root->end_column == 38);
  TEST_ASSERT(root->as.assign.target->kind == MS_AST_VARIABLE);
  TEST_ASSERT(root->as.assign.value->kind == MS_AST_ASSIGN);
  TEST_ASSERT(root->as.assign.target->line == 1);
  TEST_ASSERT(root->as.assign.target->column == 1);
  TEST_ASSERT(root->as.assign.target->end_column == 1);
  TEST_ASSERT(snapshot.length == strlen(expected));
  TEST_ASSERT(memcmp(snapshot.data, expected, snapshot.length) == 0);

  ms_buffer_destroy(&snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_postfix_chains_and_grouping_vs_tuple(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsBuffer snapshot;
  MsAstNode *root = NULL;
  const char *expected =
      "index\n"
      "  call argc=2\n"
      "    property \"field\"\n"
      "      variable \"obj\"\n"
      "    number \"1\"\n"
      "    tuple count=1\n"
      "      variable \"two\"\n"
      "  variable \"index\"\n";

  TEST_ASSERT(parse_expression_snapshot("obj.field(1, (two,))[index]\n",
                                        &arena,
                                        &diagnostics,
                                        &snapshot,
                                        &root));
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(root != NULL);
  TEST_ASSERT(root->kind == MS_AST_INDEX);
  TEST_ASSERT(root->as.index.object->kind == MS_AST_CALL);
  TEST_ASSERT(root->as.index.index->kind == MS_AST_VARIABLE);
  TEST_ASSERT(snapshot.length == strlen(expected));
  TEST_ASSERT(memcmp(snapshot.data, expected, snapshot.length) == 0);

  ms_buffer_destroy(&snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_function_map_self_and_super_shapes(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsBuffer snapshot;
  MsAstNode *root = NULL;
  const char *expected =
      "function params=2\n"
      "  param \"left\"\n"
      "  param \"right\"\n"
      "  block count=1\n"
      "    return\n"
      "      map count=2\n"
      "        entry\n"
      "          self\n"
      "          self\n"
      "        entry\n"
      "          variable \"base\"\n"
      "          super \"run\"\n";

  TEST_ASSERT(parse_expression_snapshot(
      "fn(left, right) { return {self: self, base: super.run} }\n",
      &arena,
      &diagnostics,
      &snapshot,
      &root));
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(root != NULL);
  TEST_ASSERT(root->kind == MS_AST_FUNCTION);
  TEST_ASSERT(root->as.function.parameters.count == 2);
  TEST_ASSERT(root->as.function.body != NULL);
  TEST_ASSERT(root->as.function.body->kind == MS_AST_BLOCK);
  TEST_ASSERT(snapshot.length == strlen(expected));
  TEST_ASSERT(memcmp(snapshot.data, expected, snapshot.length) == 0);

  ms_buffer_destroy(&snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

static int test_reports_invalid_assignment_and_bad_super_syntax(void) {
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsBuffer snapshot;
  const MsDiagnostic *diagnostic;

  TEST_ASSERT(!parse_expression_snapshot("a + b = c\n",
                                         &arena,
                                         &diagnostics,
                                         &snapshot,
                                         NULL));
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 1);
  diagnostic = ms_diag_list_at(&diagnostics, 0);
  TEST_ASSERT(diagnostic != NULL);
  TEST_ASSERT(strcmp(diagnostic->phase, "parse") == 0);
  TEST_ASSERT(strcmp(diagnostic->code, "MS2003") == 0);
  TEST_ASSERT(strcmp(diagnostic->message, "invalid assignment target") == 0);
  TEST_ASSERT(diagnostic->span.line == 1);
  TEST_ASSERT(diagnostic->span.column == 1);

  ms_buffer_destroy(&snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);

  TEST_ASSERT(!parse_expression_snapshot("super\n",
                                         &arena,
                                         &diagnostics,
                                         &snapshot,
                                         NULL));
  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 1);
  diagnostic = ms_diag_list_at(&diagnostics, 0);
  TEST_ASSERT(diagnostic != NULL);
  TEST_ASSERT(strcmp(diagnostic->phase, "parse") == 0);
  TEST_ASSERT(strcmp(diagnostic->code, "MS2002") == 0);
  TEST_ASSERT(strcmp(diagnostic->message,
                     "expected '.' and method name after 'super'") == 0);
  TEST_ASSERT(diagnostic->span.line == 1);
  TEST_ASSERT(diagnostic->span.column == 1);

  ms_buffer_destroy(&snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}

int main(void) {
  TEST_ASSERT(test_assignment_precedence_and_spans() == 0);
  TEST_ASSERT(test_postfix_chains_and_grouping_vs_tuple() == 0);
  TEST_ASSERT(test_function_map_self_and_super_shapes() == 0);
  TEST_ASSERT(test_reports_invalid_assignment_and_bad_super_syntax() == 0);
  return 0;
}