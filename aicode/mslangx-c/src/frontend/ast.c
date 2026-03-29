#include "ms/ast.h"

#include <stdio.h>
#include <string.h>

static int ms_ast_append_string(MsBuffer *buffer, const char *text) {
  return ms_buffer_append(buffer, text, strlen(text));
}

static int ms_ast_append_char(MsBuffer *buffer, char ch) {
  return ms_buffer_append(buffer, &ch, 1);
}

static int ms_ast_append_indent(MsBuffer *buffer, int depth) {
  int i;

  for (i = 0; i < depth; ++i) {
    if (!ms_ast_append_string(buffer, "  ")) {
      return 0;
    }
  }

  return 1;
}

static int ms_ast_append_escaped_slice(MsBuffer *buffer,
                                       const char *text,
                                       size_t length) {
  size_t i;

  for (i = 0; i < length; ++i) {
    switch (text[i]) {
      case '\\':
        if (!ms_ast_append_string(buffer, "\\\\")) {
          return 0;
        }
        break;
      case '"':
        if (!ms_ast_append_string(buffer, "\\\"")) {
          return 0;
        }
        break;
      case '\n':
        if (!ms_ast_append_string(buffer, "\\n")) {
          return 0;
        }
        break;
      case '\t':
        if (!ms_ast_append_string(buffer, "\\t")) {
          return 0;
        }
        break;
      case '\r':
        if (!ms_ast_append_string(buffer, "\\r")) {
          return 0;
        }
        break;
      default:
        if (!ms_ast_append_char(buffer, text[i])) {
          return 0;
        }
        break;
    }
  }

  return 1;
}

static int ms_ast_append_quoted_slice(MsBuffer *buffer,
                                      const char *text,
                                      size_t length) {
  return ms_ast_append_char(buffer, '"') &&
         ms_ast_append_escaped_slice(buffer, text, length) &&
         ms_ast_append_char(buffer, '"');
}

static int ms_ast_append_line(MsBuffer *buffer, int depth, const char *text) {
  return ms_ast_append_indent(buffer, depth) &&
         ms_ast_append_string(buffer, text) &&
         ms_ast_append_char(buffer, '\n');
}

static int ms_ast_append_line_with_token(MsBuffer *buffer,
                                         int depth,
                                         const char *label,
                                         MsToken token) {
  return ms_ast_append_indent(buffer, depth) &&
         ms_ast_append_string(buffer, label) &&
         ms_ast_append_char(buffer, ' ') &&
         ms_ast_append_quoted_slice(buffer, token.start, token.length) &&
         ms_ast_append_char(buffer, '\n');
}

static int ms_ast_append_line_with_count(MsBuffer *buffer,
                                         int depth,
                                         const char *label,
                                         size_t count) {
  char line[64];
  int written;

  written = snprintf(line, sizeof(line), "%s count=%zu", label, count);
  if (written < 0 || (size_t) written >= sizeof(line)) {
    return 0;
  }

  return ms_ast_append_line(buffer, depth, line);
}

static int ms_ast_append_line_with_argc(MsBuffer *buffer,
                                        int depth,
                                        size_t argc) {
  char line[64];
  int written;

  written = snprintf(line, sizeof(line), "call argc=%zu", argc);
  if (written < 0 || (size_t) written >= sizeof(line)) {
    return 0;
  }

  return ms_ast_append_line(buffer, depth, line);
}

static int ms_ast_append_line_with_params(MsBuffer *buffer,
                                          int depth,
                                          size_t count) {
  char line[64];
  int written;

  written = snprintf(line, sizeof(line), "function params=%zu", count);
  if (written < 0 || (size_t) written >= sizeof(line)) {
    return 0;
  }

  return ms_ast_append_line(buffer, depth, line);
}

static int ms_ast_token_present(MsToken token) {
  return token.start != NULL || token.length > 0;
}

static int ms_ast_append_module_path(MsBuffer *buffer, MsTokenArray path) {
  size_t i;

  for (i = 0; i < path.count; ++i) {
    if (i > 0 && !ms_ast_append_char(buffer, '.')) {
      return 0;
    }
    if (!ms_ast_append_escaped_slice(buffer,
                                     path.items[i].start,
                                     path.items[i].length)) {
      return 0;
    }
  }

  return 1;
}

static int ms_ast_append_import_line(MsBuffer *buffer,
                                     int depth,
                                     const MsAstNode *node) {
  if (!ms_ast_append_indent(buffer, depth) ||
      !ms_ast_append_string(buffer, "import path=\"") ||
      !ms_ast_append_module_path(buffer, node->as.import_stmt.path) ||
      !ms_ast_append_char(buffer, '"')) {
    return 0;
  }

  if (ms_ast_token_present(node->as.import_stmt.alias) &&
      (!ms_ast_append_string(buffer, " alias=") ||
       !ms_ast_append_quoted_slice(buffer,
                                   node->as.import_stmt.alias.start,
                                   node->as.import_stmt.alias.length))) {
    return 0;
  }

  return ms_ast_append_char(buffer, '\n');
}

static int ms_ast_append_from_import_line(MsBuffer *buffer,
                                          int depth,
                                          const MsAstNode *node) {
  if (!ms_ast_append_indent(buffer, depth) ||
      !ms_ast_append_string(buffer, "from_import path=\"") ||
      !ms_ast_append_module_path(buffer, node->as.from_import_stmt.path) ||
      !ms_ast_append_string(buffer, "\" name=") ||
      !ms_ast_append_quoted_slice(buffer,
                                  node->as.from_import_stmt.name.start,
                                  node->as.from_import_stmt.name.length)) {
    return 0;
  }

  if (ms_ast_token_present(node->as.from_import_stmt.alias) &&
      (!ms_ast_append_string(buffer, " alias=") ||
       !ms_ast_append_quoted_slice(buffer,
                                   node->as.from_import_stmt.alias.start,
                                   node->as.from_import_stmt.alias.length))) {
    return 0;
  }

  return ms_ast_append_char(buffer, '\n');
}

static int ms_ast_dump_node(MsBuffer *buffer, const MsAstNode *node, int depth) {
  size_t i;

  if (node == NULL) {
    return ms_ast_append_line(buffer, depth, "<null>");
  }

  switch (node->kind) {
    case MS_AST_PROGRAM:
      if (!ms_ast_append_line_with_count(buffer,
                                         depth,
                                         "program",
                                         node->as.program.declarations.count)) {
        return 0;
      }
      for (i = 0; i < node->as.program.declarations.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.program.declarations.items[i],
                              depth + 1)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_VAR_DECL:
      return ms_ast_append_line_with_token(buffer,
                                           depth,
                                           "var",
                                           node->as.var_decl.name) &&
             ms_ast_dump_node(buffer, node->as.var_decl.initializer, depth + 1);
    case MS_AST_FUNCTION_DECL:
      {
        char count_text[32];
        int written = snprintf(count_text,
                               sizeof(count_text),
                               "%zu",
                               node->as.function_decl.parameters.count);
        if (written < 0 || (size_t) written >= sizeof(count_text)) {
          return 0;
        }
        if (!ms_ast_append_indent(buffer, depth) ||
            !ms_ast_append_string(buffer, "function_decl ") ||
            !ms_ast_append_quoted_slice(buffer,
                                        node->as.function_decl.name.start,
                                        node->as.function_decl.name.length) ||
            !ms_ast_append_string(buffer, " params=") ||
            !ms_ast_append_string(buffer, count_text) ||
            !ms_ast_append_char(buffer, '\n')) {
          return 0;
        }
      }
      for (i = 0; i < node->as.function_decl.parameters.count; ++i) {
        if (!ms_ast_append_line_with_token(buffer,
                                           depth + 1,
                                           "param",
                                           node->as.function_decl.parameters.items[i])) {
          return 0;
        }
      }
      return ms_ast_dump_node(buffer, node->as.function_decl.body, depth + 1);
    case MS_AST_CLASS_DECL:
      if (!ms_ast_append_line_with_token(buffer,
                                         depth,
                                         "class_decl",
                                         node->as.class_decl.name)) {
        return 0;
      }
      if (ms_ast_token_present(node->as.class_decl.superclass) &&
          !ms_ast_append_line_with_token(buffer,
                                         depth + 1,
                                         "superclass",
                                         node->as.class_decl.superclass)) {
        return 0;
      }
      if (!ms_ast_append_line_with_count(buffer,
                                         depth + 1,
                                         "methods",
                                         node->as.class_decl.methods.count)) {
        return 0;
      }
      for (i = 0; i < node->as.class_decl.methods.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.class_decl.methods.items[i],
                              depth + 2)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_IMPORT_STMT:
      return ms_ast_append_import_line(buffer, depth, node);
    case MS_AST_FROM_IMPORT_STMT:
      return ms_ast_append_from_import_line(buffer, depth, node);
    case MS_AST_EXPR_STMT:
      return ms_ast_append_line(buffer, depth, "expr_stmt") &&
             ms_ast_dump_node(buffer, node->as.expr_stmt.expression, depth + 1);
    case MS_AST_PRINT_STMT:
      return ms_ast_append_line(buffer, depth, "print") &&
             ms_ast_dump_node(buffer, node->as.print_stmt.expression, depth + 1);
    case MS_AST_RETURN_STMT:
      if (!ms_ast_append_line(buffer, depth, "return")) {
        return 0;
      }
      if (node->as.return_stmt.value != NULL) {
        return ms_ast_dump_node(buffer, node->as.return_stmt.value, depth + 1);
      }
      return 1;
    case MS_AST_BREAK_STMT:
      return ms_ast_append_line(buffer, depth, "break");
    case MS_AST_CONTINUE_STMT:
      return ms_ast_append_line(buffer, depth, "continue");
    case MS_AST_IF_STMT:
      if (!ms_ast_append_line(buffer, depth, "if") ||
          !ms_ast_dump_node(buffer, node->as.if_stmt.condition, depth + 1) ||
          !ms_ast_dump_node(buffer, node->as.if_stmt.then_branch, depth + 1)) {
        return 0;
      }
      if (node->as.if_stmt.else_branch != NULL) {
        return ms_ast_dump_node(buffer, node->as.if_stmt.else_branch, depth + 1);
      }
      return 1;
    case MS_AST_WHILE_STMT:
      return ms_ast_append_line(buffer, depth, "while") &&
             ms_ast_dump_node(buffer, node->as.while_stmt.condition, depth + 1) &&
             ms_ast_dump_node(buffer, node->as.while_stmt.body, depth + 1);
    case MS_AST_FOR_STMT:
      if (!ms_ast_append_line(buffer, depth, "for") ||
          !ms_ast_append_line(buffer, depth + 1, "init")) {
        return 0;
      }
      if (node->as.for_stmt.initializer != NULL) {
        if (!ms_ast_dump_node(buffer, node->as.for_stmt.initializer, depth + 2)) {
          return 0;
        }
      } else if (!ms_ast_append_line(buffer, depth + 2, "<none>")) {
        return 0;
      }
      if (!ms_ast_append_line(buffer, depth + 1, "condition")) {
        return 0;
      }
      if (node->as.for_stmt.condition != NULL) {
        if (!ms_ast_dump_node(buffer, node->as.for_stmt.condition, depth + 2)) {
          return 0;
        }
      } else if (!ms_ast_append_line(buffer, depth + 2, "<none>")) {
        return 0;
      }
      if (!ms_ast_append_line(buffer, depth + 1, "increment")) {
        return 0;
      }
      if (node->as.for_stmt.increment != NULL) {
        if (!ms_ast_dump_node(buffer, node->as.for_stmt.increment, depth + 2)) {
          return 0;
        }
      } else if (!ms_ast_append_line(buffer, depth + 2, "<none>")) {
        return 0;
      }
      return ms_ast_dump_node(buffer, node->as.for_stmt.body, depth + 1);
    case MS_AST_LITERAL:
      switch (node->as.literal.token.kind) {
        case MS_TOKEN_NUMBER:
          return ms_ast_append_line_with_token(buffer,
                                               depth,
                                               "number",
                                               node->as.literal.token);
        case MS_TOKEN_STRING:
          return ms_ast_append_line_with_token(buffer,
                                               depth,
                                               "string",
                                               node->as.literal.token);
        case MS_TOKEN_TRUE:
          return ms_ast_append_line(buffer, depth, "true");
        case MS_TOKEN_FALSE:
          return ms_ast_append_line(buffer, depth, "false");
        case MS_TOKEN_NIL:
          return ms_ast_append_line(buffer, depth, "nil");
        default:
          return ms_ast_append_line(buffer, depth, "literal");
      }
    case MS_AST_VARIABLE:
      return ms_ast_append_line_with_token(buffer,
                                           depth,
                                           "variable",
                                           node->as.variable.name);
    case MS_AST_SELF:
      return ms_ast_append_line(buffer, depth, "self");
    case MS_AST_SUPER:
      return ms_ast_append_line_with_token(buffer,
                                           depth,
                                           "super",
                                           node->as.super_expr.method);
    case MS_AST_UNARY:
      return ms_ast_append_indent(buffer, depth) &&
             ms_ast_append_string(buffer, "unary ") &&
             ms_ast_append_quoted_slice(buffer,
                                        node->as.unary.op.start,
                                        node->as.unary.op.length) &&
             ms_ast_append_char(buffer, '\n') &&
             ms_ast_dump_node(buffer, node->as.unary.operand, depth + 1);
    case MS_AST_BINARY:
      return ms_ast_append_indent(buffer, depth) &&
             ms_ast_append_string(buffer, "binary ") &&
             ms_ast_append_quoted_slice(buffer,
                                        node->as.binary.op.start,
                                        node->as.binary.op.length) &&
             ms_ast_append_char(buffer, '\n') &&
             ms_ast_dump_node(buffer, node->as.binary.left, depth + 1) &&
             ms_ast_dump_node(buffer, node->as.binary.right, depth + 1);
    case MS_AST_LOGICAL:
      return ms_ast_append_indent(buffer, depth) &&
             ms_ast_append_string(buffer, "logical ") &&
             ms_ast_append_quoted_slice(buffer,
                                        node->as.logical.op.start,
                                        node->as.logical.op.length) &&
             ms_ast_append_char(buffer, '\n') &&
             ms_ast_dump_node(buffer, node->as.logical.left, depth + 1) &&
             ms_ast_dump_node(buffer, node->as.logical.right, depth + 1);
    case MS_AST_ASSIGN:
      return ms_ast_append_line(buffer, depth, "assign") &&
             ms_ast_dump_node(buffer, node->as.assign.target, depth + 1) &&
             ms_ast_dump_node(buffer, node->as.assign.value, depth + 1);
    case MS_AST_CALL:
      if (!ms_ast_append_line_with_argc(buffer,
                                        depth,
                                        node->as.call.arguments.count) ||
          !ms_ast_dump_node(buffer, node->as.call.callee, depth + 1)) {
        return 0;
      }
      for (i = 0; i < node->as.call.arguments.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.call.arguments.items[i],
                              depth + 1)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_PROPERTY:
      return ms_ast_append_line_with_token(buffer,
                                           depth,
                                           "property",
                                           node->as.property.name) &&
             ms_ast_dump_node(buffer, node->as.property.object, depth + 1);
    case MS_AST_INDEX:
      return ms_ast_append_line(buffer, depth, "index") &&
             ms_ast_dump_node(buffer, node->as.index.object, depth + 1) &&
             ms_ast_dump_node(buffer, node->as.index.index, depth + 1);
    case MS_AST_LIST:
      if (!ms_ast_append_line_with_count(buffer,
                                         depth,
                                         "list",
                                         node->as.list.elements.count)) {
        return 0;
      }
      for (i = 0; i < node->as.list.elements.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.list.elements.items[i],
                              depth + 1)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_TUPLE:
      if (!ms_ast_append_line_with_count(buffer,
                                         depth,
                                         "tuple",
                                         node->as.tuple.elements.count)) {
        return 0;
      }
      for (i = 0; i < node->as.tuple.elements.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.tuple.elements.items[i],
                              depth + 1)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_MAP:
      if (!ms_ast_append_line_with_count(buffer,
                                         depth,
                                         "map",
                                         node->as.map.entries.count)) {
        return 0;
      }
      for (i = 0; i < node->as.map.entries.count; ++i) {
        if (!ms_ast_append_line(buffer, depth + 1, "entry") ||
            !ms_ast_dump_node(buffer,
                              node->as.map.entries.items[i].key,
                              depth + 2) ||
            !ms_ast_dump_node(buffer,
                              node->as.map.entries.items[i].value,
                              depth + 2)) {
          return 0;
        }
      }
      return 1;
    case MS_AST_FUNCTION:
      if (!ms_ast_append_line_with_params(buffer,
                                          depth,
                                          node->as.function.parameters.count)) {
        return 0;
      }
      for (i = 0; i < node->as.function.parameters.count; ++i) {
        if (!ms_ast_append_line_with_token(buffer,
                                           depth + 1,
                                           "param",
                                           node->as.function.parameters.items[i])) {
          return 0;
        }
      }
      return ms_ast_dump_node(buffer, node->as.function.body, depth + 1);
    case MS_AST_BLOCK:
      if (!ms_ast_append_line_with_count(buffer,
                                         depth,
                                         "block",
                                         node->as.block.statements.count)) {
        return 0;
      }
      for (i = 0; i < node->as.block.statements.count; ++i) {
        if (!ms_ast_dump_node(buffer,
                              node->as.block.statements.items[i],
                              depth + 1)) {
          return 0;
        }
      }
      return 1;
  }

  return 0;
}

const char *ms_ast_kind_name(MsAstKind kind) {
  switch (kind) {
    case MS_AST_PROGRAM:
      return "program";
    case MS_AST_VAR_DECL:
      return "var_decl";
    case MS_AST_FUNCTION_DECL:
      return "function_decl";
    case MS_AST_CLASS_DECL:
      return "class_decl";
    case MS_AST_IMPORT_STMT:
      return "import_stmt";
    case MS_AST_FROM_IMPORT_STMT:
      return "from_import_stmt";
    case MS_AST_EXPR_STMT:
      return "expr_stmt";
    case MS_AST_PRINT_STMT:
      return "print_stmt";
    case MS_AST_RETURN_STMT:
      return "return_stmt";
    case MS_AST_BREAK_STMT:
      return "break_stmt";
    case MS_AST_CONTINUE_STMT:
      return "continue_stmt";
    case MS_AST_IF_STMT:
      return "if_stmt";
    case MS_AST_WHILE_STMT:
      return "while_stmt";
    case MS_AST_FOR_STMT:
      return "for_stmt";
    case MS_AST_LITERAL:
      return "literal";
    case MS_AST_VARIABLE:
      return "variable";
    case MS_AST_SELF:
      return "self";
    case MS_AST_SUPER:
      return "super";
    case MS_AST_UNARY:
      return "unary";
    case MS_AST_BINARY:
      return "binary";
    case MS_AST_LOGICAL:
      return "logical";
    case MS_AST_ASSIGN:
      return "assign";
    case MS_AST_CALL:
      return "call";
    case MS_AST_PROPERTY:
      return "property";
    case MS_AST_INDEX:
      return "index";
    case MS_AST_LIST:
      return "list";
    case MS_AST_TUPLE:
      return "tuple";
    case MS_AST_MAP:
      return "map";
    case MS_AST_FUNCTION:
      return "function";
    case MS_AST_BLOCK:
      return "block";
  }

  return "unknown";
}

int ms_ast_dump(MsBuffer *buffer, const MsAstNode *node) {
  if (buffer == NULL) {
    return 0;
  }

  ms_buffer_clear(buffer);
  return ms_ast_dump_node(buffer, node, 0);
}