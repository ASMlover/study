#include "ms/lexer.h"

#include <stddef.h>
#include <string.h>

static int ms_lexer_is_at_end(const MsLexer *lexer) {
  return *lexer->current == '\0';
}

static char ms_lexer_peek(const MsLexer *lexer) {
  return *lexer->current;
}

static char ms_lexer_peek_next(const MsLexer *lexer) {
  if (ms_lexer_is_at_end(lexer)) {
    return '\0';
  }

  return lexer->current[1];
}

static char ms_lexer_advance(MsLexer *lexer) {
  char ch = *lexer->current;

  if (ch == '\0') {
    return ch;
  }

  lexer->current += 1;
  if (ch == '\n') {
    lexer->line += 1;
    lexer->column = 1;
  } else {
    lexer->column += 1;
  }

  return ch;
}

static int ms_lexer_match(MsLexer *lexer, char expected) {
  if (ms_lexer_is_at_end(lexer) || *lexer->current != expected) {
    return 0;
  }

  ms_lexer_advance(lexer);
  return 1;
}

static int ms_lexer_is_alpha(char ch) {
  return (ch >= 'a' && ch <= 'z') ||
         (ch >= 'A' && ch <= 'Z') ||
         ch == '_';
}

static int ms_lexer_is_digit(char ch) {
  return ch >= '0' && ch <= '9';
}

static int ms_lexer_is_alpha_numeric(char ch) {
  return ms_lexer_is_alpha(ch) || ms_lexer_is_digit(ch);
}

static void ms_lexer_skip_horizontal_whitespace(MsLexer *lexer) {
  while (!ms_lexer_is_at_end(lexer)) {
    char ch = ms_lexer_peek(lexer);

    if (ch == ' ' || ch == '\t' || ch == '\r') {
      ms_lexer_advance(lexer);
      continue;
    }

    break;
  }
}

static MsToken ms_lexer_make_token(MsTokenKind kind,
                                   const char *start,
                                   size_t length,
                                   int line,
                                   int column,
                                   int end_column) {
  MsToken token;

  token.kind = kind;
  token.start = start;
  token.length = length;
  token.line = line;
  token.column = column;
  token.end_column = end_column;
  return token;
}

static void ms_lexer_append_diagnostic(MsLexer *lexer,
                                       const char *code,
                                       const char *message,
                                       int line,
                                       int column,
                                       int length) {
  MsDiagnostic diagnostic;

  if (lexer->diagnostics == NULL) {
    return;
  }

  diagnostic.phase = "lex";
  diagnostic.code = code;
  diagnostic.message = message;
  diagnostic.span.file = lexer->file;
  diagnostic.span.line = line;
  diagnostic.span.column = column;
  diagnostic.span.length = length;
  ms_diag_list_append(lexer->diagnostics, &diagnostic);
}

static MsTokenKind ms_lexer_identifier_kind(const char *start, size_t length) {
  if (length == 2) {
    if (memcmp(start, "fn", 2) == 0) {
      return MS_TOKEN_FN;
    }
    if (memcmp(start, "as", 2) == 0) {
      return MS_TOKEN_AS;
    }
    if (memcmp(start, "if", 2) == 0) {
      return MS_TOKEN_IF;
    }
    if (memcmp(start, "or", 2) == 0) {
      return MS_TOKEN_OR;
    }
  }

  if (length == 3) {
    if (memcmp(start, "var", 3) == 0) {
      return MS_TOKEN_VAR;
    }
    if (memcmp(start, "for", 3) == 0) {
      return MS_TOKEN_FOR;
    }
    if (memcmp(start, "and", 3) == 0) {
      return MS_TOKEN_AND;
    }
    if (memcmp(start, "nil", 3) == 0) {
      return MS_TOKEN_NIL;
    }
  }

  if (length == 4) {
    if (memcmp(start, "self", 4) == 0) {
      return MS_TOKEN_SELF;
    }
    if (memcmp(start, "from", 4) == 0) {
      return MS_TOKEN_FROM;
    }
    if (memcmp(start, "else", 4) == 0) {
      return MS_TOKEN_ELSE;
    }
    if (memcmp(start, "true", 4) == 0) {
      return MS_TOKEN_TRUE;
    }
  }

  if (length == 5) {
    if (memcmp(start, "class", 5) == 0) {
      return MS_TOKEN_CLASS;
    }
    if (memcmp(start, "super", 5) == 0) {
      return MS_TOKEN_SUPER;
    }
    if (memcmp(start, "print", 5) == 0) {
      return MS_TOKEN_PRINT;
    }
    if (memcmp(start, "while", 5) == 0) {
      return MS_TOKEN_WHILE;
    }
    if (memcmp(start, "break", 5) == 0) {
      return MS_TOKEN_BREAK;
    }
    if (memcmp(start, "false", 5) == 0) {
      return MS_TOKEN_FALSE;
    }
  }

  if (length == 6) {
    if (memcmp(start, "return", 6) == 0) {
      return MS_TOKEN_RETURN;
    }
    if (memcmp(start, "import", 6) == 0) {
      return MS_TOKEN_IMPORT;
    }
  }

  if (length == 8 && memcmp(start, "continue", 8) == 0) {
    return MS_TOKEN_CONTINUE;
  }

  return MS_TOKEN_IDENTIFIER;
}

static MsToken ms_lexer_scan_identifier(MsLexer *lexer,
                                        const char *start,
                                        int line,
                                        int column) {
  while (ms_lexer_is_alpha_numeric(ms_lexer_peek(lexer))) {
    ms_lexer_advance(lexer);
  }

  return ms_lexer_make_token(ms_lexer_identifier_kind(start,
                                                      (size_t) (lexer->current - start)),
                             start,
                             (size_t) (lexer->current - start),
                             line,
                             column,
                             lexer->column - 1);
}

static MsToken ms_lexer_scan_number(MsLexer *lexer,
                                    const char *start,
                                    int line,
                                    int column) {
  while (ms_lexer_is_digit(ms_lexer_peek(lexer))) {
    ms_lexer_advance(lexer);
  }

  if (ms_lexer_peek(lexer) == '.' && ms_lexer_is_digit(ms_lexer_peek_next(lexer))) {
    ms_lexer_advance(lexer);
    while (ms_lexer_is_digit(ms_lexer_peek(lexer))) {
      ms_lexer_advance(lexer);
    }
  }

  return ms_lexer_make_token(MS_TOKEN_NUMBER,
                             start,
                             (size_t) (lexer->current - start),
                             line,
                             column,
                             lexer->column - 1);
}

static int ms_lexer_scan_string(MsLexer *lexer,
                                const char *start,
                                int line,
                                int column,
                                MsToken *out_token) {
  while (!ms_lexer_is_at_end(lexer)) {
    char ch = ms_lexer_peek(lexer);

    if (ch == '"') {
      ms_lexer_advance(lexer);
      *out_token = ms_lexer_make_token(MS_TOKEN_STRING,
                                       start,
                                       (size_t) (lexer->current - start),
                                       line,
                                       column,
                                       lexer->column - 1);
      return 1;
    }

    if (ch == '\n') {
      ms_lexer_append_diagnostic(lexer,
                                 "MS1001",
                                 "unterminated string literal",
                                 line,
                                 column,
                                 (int) (lexer->current - start));
      return 0;
    }

    if (ch == '\\') {
      ms_lexer_advance(lexer);
      if (!ms_lexer_is_at_end(lexer) && ms_lexer_peek(lexer) != '\n') {
        ms_lexer_advance(lexer);
      }
      continue;
    }

    ms_lexer_advance(lexer);
  }

  ms_lexer_append_diagnostic(lexer,
                             "MS1001",
                             "unterminated string literal",
                             line,
                             column,
                             (int) (lexer->current - start));
  return 0;
}

void ms_lexer_init(MsLexer *lexer,
                   const char *file,
                   const char *source,
                   MsDiagnosticList *diagnostics) {
  lexer->file = file;
  lexer->source = source != NULL ? source : "";
  lexer->current = lexer->source;
  lexer->line = 1;
  lexer->column = 1;
  lexer->diagnostics = diagnostics;
}

MsToken ms_lexer_next(MsLexer *lexer) {
  for (;;) {
    MsToken token;
    const char *start;
    int line;
    int column;
    char ch;

    ms_lexer_skip_horizontal_whitespace(lexer);

    if (ms_lexer_is_at_end(lexer)) {
      return ms_lexer_make_token(MS_TOKEN_EOF,
                                 lexer->current,
                                 0,
                                 lexer->line,
                                 lexer->column,
                                 lexer->column);
    }

    start = lexer->current;
    line = lexer->line;
    column = lexer->column;
    ch = ms_lexer_advance(lexer);

    switch (ch) {
      case '(':
        return ms_lexer_make_token(MS_TOKEN_LEFT_PAREN, start, 1, line, column, column);
      case ')':
        return ms_lexer_make_token(MS_TOKEN_RIGHT_PAREN, start, 1, line, column, column);
      case '{':
        return ms_lexer_make_token(MS_TOKEN_LEFT_BRACE, start, 1, line, column, column);
      case '}':
        return ms_lexer_make_token(MS_TOKEN_RIGHT_BRACE, start, 1, line, column, column);
      case '[':
        return ms_lexer_make_token(MS_TOKEN_LEFT_BRACKET, start, 1, line, column, column);
      case ']':
        return ms_lexer_make_token(MS_TOKEN_RIGHT_BRACKET, start, 1, line, column, column);
      case ',':
        return ms_lexer_make_token(MS_TOKEN_COMMA, start, 1, line, column, column);
      case '.':
        return ms_lexer_make_token(MS_TOKEN_DOT, start, 1, line, column, column);
      case '-':
        return ms_lexer_make_token(MS_TOKEN_MINUS, start, 1, line, column, column);
      case '+':
        return ms_lexer_make_token(MS_TOKEN_PLUS, start, 1, line, column, column);
      case ';':
        return ms_lexer_make_token(MS_TOKEN_SEMICOLON, start, 1, line, column, column);
      case '/':
        return ms_lexer_make_token(MS_TOKEN_SLASH, start, 1, line, column, column);
      case '*':
        return ms_lexer_make_token(MS_TOKEN_STAR, start, 1, line, column, column);
      case ':':
        return ms_lexer_make_token(MS_TOKEN_COLON, start, 1, line, column, column);
      case '!':
        return ms_lexer_make_token(ms_lexer_match(lexer, '=') ? MS_TOKEN_BANG_EQUAL : MS_TOKEN_BANG,
                                   start,
                                   (size_t) (lexer->current - start),
                                   line,
                                   column,
                                   lexer->column - 1);
      case '=':
        return ms_lexer_make_token(ms_lexer_match(lexer, '=') ? MS_TOKEN_EQUAL_EQUAL : MS_TOKEN_EQUAL,
                                   start,
                                   (size_t) (lexer->current - start),
                                   line,
                                   column,
                                   lexer->column - 1);
      case '<':
        return ms_lexer_make_token(ms_lexer_match(lexer, '=') ? MS_TOKEN_LESS_EQUAL : MS_TOKEN_LESS,
                                   start,
                                   (size_t) (lexer->current - start),
                                   line,
                                   column,
                                   lexer->column - 1);
      case '>':
        return ms_lexer_make_token(ms_lexer_match(lexer, '=') ? MS_TOKEN_GREATER_EQUAL : MS_TOKEN_GREATER,
                                   start,
                                   (size_t) (lexer->current - start),
                                   line,
                                   column,
                                   lexer->column - 1);
      case '\n':
        return ms_lexer_make_token(MS_TOKEN_NEWLINE, start, 1, line, column, column);
      case '"':
        if (ms_lexer_scan_string(lexer, start, line, column, &token)) {
          return token;
        }
        continue;
      default:
        if (ms_lexer_is_digit(ch)) {
          return ms_lexer_scan_number(lexer, start, line, column);
        }
        if (ms_lexer_is_alpha(ch)) {
          return ms_lexer_scan_identifier(lexer, start, line, column);
        }

        ms_lexer_append_diagnostic(lexer,
                                   "MS1002",
                                   "unexpected character",
                                   line,
                                   column,
                                   1);
        continue;
    }
  }
}

const char *ms_token_kind_name(MsTokenKind kind) {
  switch (kind) {
    case MS_TOKEN_LEFT_PAREN:
      return "LEFT_PAREN";
    case MS_TOKEN_RIGHT_PAREN:
      return "RIGHT_PAREN";
    case MS_TOKEN_LEFT_BRACE:
      return "LEFT_BRACE";
    case MS_TOKEN_RIGHT_BRACE:
      return "RIGHT_BRACE";
    case MS_TOKEN_LEFT_BRACKET:
      return "LEFT_BRACKET";
    case MS_TOKEN_RIGHT_BRACKET:
      return "RIGHT_BRACKET";
    case MS_TOKEN_COMMA:
      return "COMMA";
    case MS_TOKEN_DOT:
      return "DOT";
    case MS_TOKEN_MINUS:
      return "MINUS";
    case MS_TOKEN_PLUS:
      return "PLUS";
    case MS_TOKEN_SEMICOLON:
      return "SEMICOLON";
    case MS_TOKEN_SLASH:
      return "SLASH";
    case MS_TOKEN_STAR:
      return "STAR";
    case MS_TOKEN_COLON:
      return "COLON";
    case MS_TOKEN_BANG:
      return "BANG";
    case MS_TOKEN_BANG_EQUAL:
      return "BANG_EQUAL";
    case MS_TOKEN_EQUAL:
      return "EQUAL";
    case MS_TOKEN_EQUAL_EQUAL:
      return "EQUAL_EQUAL";
    case MS_TOKEN_GREATER:
      return "GREATER";
    case MS_TOKEN_GREATER_EQUAL:
      return "GREATER_EQUAL";
    case MS_TOKEN_LESS:
      return "LESS";
    case MS_TOKEN_LESS_EQUAL:
      return "LESS_EQUAL";
    case MS_TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case MS_TOKEN_STRING:
      return "STRING";
    case MS_TOKEN_NUMBER:
      return "NUMBER";
    case MS_TOKEN_VAR:
      return "VAR";
    case MS_TOKEN_FN:
      return "FN";
    case MS_TOKEN_RETURN:
      return "RETURN";
    case MS_TOKEN_CLASS:
      return "CLASS";
    case MS_TOKEN_SELF:
      return "SELF";
    case MS_TOKEN_SUPER:
      return "SUPER";
    case MS_TOKEN_PRINT:
      return "PRINT";
    case MS_TOKEN_IMPORT:
      return "IMPORT";
    case MS_TOKEN_FROM:
      return "FROM";
    case MS_TOKEN_AS:
      return "AS";
    case MS_TOKEN_IF:
      return "IF";
    case MS_TOKEN_ELSE:
      return "ELSE";
    case MS_TOKEN_WHILE:
      return "WHILE";
    case MS_TOKEN_FOR:
      return "FOR";
    case MS_TOKEN_BREAK:
      return "BREAK";
    case MS_TOKEN_CONTINUE:
      return "CONTINUE";
    case MS_TOKEN_AND:
      return "AND";
    case MS_TOKEN_OR:
      return "OR";
    case MS_TOKEN_TRUE:
      return "TRUE";
    case MS_TOKEN_FALSE:
      return "FALSE";
    case MS_TOKEN_NIL:
      return "NIL";
    case MS_TOKEN_NEWLINE:
      return "NEWLINE";
    case MS_TOKEN_EOF:
      return "EOF";
  }

  return "UNKNOWN";
}