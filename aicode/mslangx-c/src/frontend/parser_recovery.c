#include "parser_internal.h"

static int ms_parser_is_sync_start(MsTokenKind kind) {
  switch (kind) {
    case MS_TOKEN_CLASS:
    case MS_TOKEN_FN:
    case MS_TOKEN_VAR:
    case MS_TOKEN_IF:
    case MS_TOKEN_WHILE:
    case MS_TOKEN_FOR:
    case MS_TOKEN_RETURN:
    case MS_TOKEN_BREAK:
    case MS_TOKEN_CONTINUE:
    case MS_TOKEN_PRINT:
    case MS_TOKEN_IMPORT:
    case MS_TOKEN_FROM:
    case MS_TOKEN_LEFT_BRACE:
    case MS_TOKEN_IDENTIFIER:
    case MS_TOKEN_STRING:
    case MS_TOKEN_NUMBER:
    case MS_TOKEN_TRUE:
    case MS_TOKEN_FALSE:
    case MS_TOKEN_NIL:
    case MS_TOKEN_SELF:
    case MS_TOKEN_SUPER:
    case MS_TOKEN_LEFT_PAREN:
    case MS_TOKEN_LEFT_BRACKET:
    case MS_TOKEN_BANG:
    case MS_TOKEN_MINUS:
      return 1;
    default:
      return 0;
  }
}

void ms_parser_synchronize(MsParser *parser) {
  if (parser == NULL) {
    return;
  }

  while (!ms_parser_check(parser, MS_TOKEN_EOF)) {
    if (parser->previous.kind == MS_TOKEN_SEMICOLON ||
        parser->previous.kind == MS_TOKEN_NEWLINE) {
      return;
    }
    if (ms_parser_check(parser, MS_TOKEN_RIGHT_BRACE) ||
        ms_parser_is_sync_start(parser->current.kind)) {
      return;
    }

    ms_parser_advance(parser);
  }
}