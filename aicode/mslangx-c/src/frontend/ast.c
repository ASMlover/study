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

static int ms_ast_dump_node(MsBuffer *buffer, const MsAstNode *node, int depth) {
  size_t i;

  if (node == NULL) {
    return ms_ast_append_line(buffer, depth, "<null>");
  }

  switch (node->kind) {
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
      return ms_ast_append_indent(buffer, depth) &&
             ms_ast_append_string(buffer, "block raw=") &&
             ms_ast_append_quoted_slice(buffer,
                                        node->as.block.raw_start,
                                        node->as.block.raw_length) &&
             ms_ast_append_char(buffer, '\n');
  }

  return 0;
}

const char *ms_ast_kind_name(MsAstKind kind) {
  switch (kind) {
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