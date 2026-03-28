#ifndef MS_LEXER_H_
#define MS_LEXER_H_

#include "ms/diag.h"
#include "ms/token.h"

typedef struct MsLexer {
  const char *file;
  const char *source;
  const char *current;
  int line;
  int column;
  MsDiagnosticList *diagnostics;
} MsLexer;

void ms_lexer_init(MsLexer *lexer,
                   const char *file,
                   const char *source,
                   MsDiagnosticList *diagnostics);
MsToken ms_lexer_next(MsLexer *lexer);

#endif