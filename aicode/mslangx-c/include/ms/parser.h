#ifndef MS_PARSER_H_
#define MS_PARSER_H_

#include <stddef.h>

#include "ms/arena.h"
#include "ms/ast.h"
#include "ms/diag.h"
#include "ms/lexer.h"

typedef struct MsParser {
  const char *file;
  MsArena *arena;
  MsDiagnosticList *diagnostics;
  MsLexer lexer;
  MsToken current;
  MsToken previous;
  size_t next_node_id;
  int had_error;
} MsParser;

void ms_parser_init(MsParser *parser,
                    const char *file,
                    const char *source,
                    MsArena *arena,
                    MsDiagnosticList *diagnostics);
MsAstNode *ms_parser_parse_expression(MsParser *parser);
MsAstNode *ms_parser_parse_expression_prec(MsParser *parser, int min_precedence);
MsAstNode *ms_parse_expression(const char *file,
                               const char *source,
                               MsArena *arena,
                               MsDiagnosticList *diagnostics);
MsAstNode *ms_parse_source(const char *file,
                           const char *source,
                           MsArena *arena,
                           MsDiagnosticList *diagnostics);

#endif