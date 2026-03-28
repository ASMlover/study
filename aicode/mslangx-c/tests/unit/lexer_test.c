#include <stddef.h>
#include <string.h>

#include "ms/buffer.h"
#include "ms/diag.h"
#include "ms/lexer.h"
#include "ms/token.h"

#include "test_assert.h"

static int token_text_equals(MsToken token, const char *text) {
  size_t length = strlen(text);

  return token.length == length && memcmp(token.start, text, length) == 0;
}

static int collect_tokens(const char *source,
                          MsBuffer *token_bytes,
                          MsDiagnosticList *diagnostics) {
  MsLexer lexer;
  size_t guard = 0;

  ms_buffer_init(token_bytes);
  ms_diag_list_init(diagnostics);
  ms_lexer_init(&lexer, "<unit>", source, diagnostics);

  for (;;) {
    MsToken token = ms_lexer_next(&lexer);

    TEST_ASSERT(ms_buffer_append(token_bytes, &token, sizeof(token)));
    guard += 1;
    TEST_ASSERT(guard < 256);

    if (token.kind == MS_TOKEN_EOF) {
      break;
    }
  }

  return 0;
}

static int test_tracks_spans_and_newlines(void) {
  MsBuffer token_bytes;
  MsDiagnosticList diagnostics;
  const MsToken *tokens;

  TEST_ASSERT(collect_tokens("foo = 3.14\n\"hi\\n\"\n<= >=\n",
                             &token_bytes,
                             &diagnostics) == 0);
  tokens = (const MsToken *) token_bytes.data;

  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(token_bytes.length / sizeof(MsToken) == 10);

  TEST_ASSERT(tokens[0].kind == MS_TOKEN_IDENTIFIER);
  TEST_ASSERT(token_text_equals(tokens[0], "foo"));
  TEST_ASSERT(tokens[0].line == 1);
  TEST_ASSERT(tokens[0].column == 1);
  TEST_ASSERT(tokens[0].end_column == 3);

  TEST_ASSERT(tokens[1].kind == MS_TOKEN_EQUAL);
  TEST_ASSERT(tokens[1].line == 1);
  TEST_ASSERT(tokens[1].column == 5);
  TEST_ASSERT(tokens[1].end_column == 5);

  TEST_ASSERT(tokens[2].kind == MS_TOKEN_NUMBER);
  TEST_ASSERT(token_text_equals(tokens[2], "3.14"));
  TEST_ASSERT(tokens[2].line == 1);
  TEST_ASSERT(tokens[2].column == 7);
  TEST_ASSERT(tokens[2].end_column == 10);

  TEST_ASSERT(tokens[3].kind == MS_TOKEN_NEWLINE);
  TEST_ASSERT(tokens[3].line == 1);
  TEST_ASSERT(tokens[3].column == 11);
  TEST_ASSERT(tokens[3].end_column == 11);

  TEST_ASSERT(tokens[4].kind == MS_TOKEN_STRING);
  TEST_ASSERT(token_text_equals(tokens[4], "\"hi\\n\""));
  TEST_ASSERT(tokens[4].line == 2);
  TEST_ASSERT(tokens[4].column == 1);
  TEST_ASSERT(tokens[4].end_column == 6);

  TEST_ASSERT(tokens[5].kind == MS_TOKEN_NEWLINE);
  TEST_ASSERT(tokens[5].line == 2);
  TEST_ASSERT(tokens[5].column == 7);
  TEST_ASSERT(tokens[5].end_column == 7);

  TEST_ASSERT(tokens[6].kind == MS_TOKEN_LESS_EQUAL);
  TEST_ASSERT(tokens[6].line == 3);
  TEST_ASSERT(tokens[6].column == 1);
  TEST_ASSERT(tokens[6].end_column == 2);

  TEST_ASSERT(tokens[7].kind == MS_TOKEN_GREATER_EQUAL);
  TEST_ASSERT(tokens[7].line == 3);
  TEST_ASSERT(tokens[7].column == 4);
  TEST_ASSERT(tokens[7].end_column == 5);

  TEST_ASSERT(tokens[8].kind == MS_TOKEN_NEWLINE);
  TEST_ASSERT(tokens[8].line == 3);
  TEST_ASSERT(tokens[8].column == 6);
  TEST_ASSERT(tokens[8].end_column == 6);

  TEST_ASSERT(tokens[9].kind == MS_TOKEN_EOF);
  TEST_ASSERT(tokens[9].line == 4);
  TEST_ASSERT(tokens[9].column == 1);
  TEST_ASSERT(tokens[9].end_column == 1);

  ms_diag_list_destroy(&diagnostics);
  ms_buffer_destroy(&token_bytes);
  return 0;
}

static int test_recognizes_basic_punctuation(void) {
  static const MsTokenKind expected[] = {
    MS_TOKEN_LEFT_PAREN,
    MS_TOKEN_RIGHT_PAREN,
    MS_TOKEN_LEFT_BRACE,
    MS_TOKEN_RIGHT_BRACE,
    MS_TOKEN_LEFT_BRACKET,
    MS_TOKEN_RIGHT_BRACKET,
    MS_TOKEN_COMMA,
    MS_TOKEN_DOT,
    MS_TOKEN_SEMICOLON,
    MS_TOKEN_COLON,
    MS_TOKEN_PLUS,
    MS_TOKEN_MINUS,
    MS_TOKEN_STAR,
    MS_TOKEN_SLASH,
    MS_TOKEN_BANG,
    MS_TOKEN_BANG_EQUAL,
    MS_TOKEN_EQUAL_EQUAL,
    MS_TOKEN_LESS,
    MS_TOKEN_LESS_EQUAL,
    MS_TOKEN_GREATER,
    MS_TOKEN_GREATER_EQUAL,
    MS_TOKEN_NEWLINE,
    MS_TOKEN_EOF
  };
  MsBuffer token_bytes;
  MsDiagnosticList diagnostics;
  const MsToken *tokens;
  size_t i;

  TEST_ASSERT(collect_tokens("(){}[],.;:+-*/! != == < <= > >=\n",
                             &token_bytes,
                             &diagnostics) == 0);
  tokens = (const MsToken *) token_bytes.data;

  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 0);
  TEST_ASSERT(token_bytes.length / sizeof(MsToken) ==
              sizeof(expected) / sizeof(expected[0]));

  for (i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
    TEST_ASSERT(tokens[i].kind == expected[i]);
  }

  ms_diag_list_destroy(&diagnostics);
  ms_buffer_destroy(&token_bytes);
  return 0;
}

static int test_reports_invalid_character_and_recovers(void) {
  MsBuffer token_bytes;
  MsDiagnosticList diagnostics;
  const MsToken *tokens;
  const MsDiagnostic *diagnostic;

  TEST_ASSERT(collect_tokens("var x = @\nprint x\n",
                             &token_bytes,
                             &diagnostics) == 0);
  tokens = (const MsToken *) token_bytes.data;

  TEST_ASSERT(token_bytes.length / sizeof(MsToken) == 8);
  TEST_ASSERT(tokens[0].kind == MS_TOKEN_VAR);
  TEST_ASSERT(tokens[1].kind == MS_TOKEN_IDENTIFIER);
  TEST_ASSERT(tokens[2].kind == MS_TOKEN_EQUAL);
  TEST_ASSERT(tokens[3].kind == MS_TOKEN_NEWLINE);
  TEST_ASSERT(tokens[4].kind == MS_TOKEN_PRINT);
  TEST_ASSERT(tokens[5].kind == MS_TOKEN_IDENTIFIER);
  TEST_ASSERT(tokens[6].kind == MS_TOKEN_NEWLINE);
  TEST_ASSERT(tokens[7].kind == MS_TOKEN_EOF);

  TEST_ASSERT(ms_diag_list_count(&diagnostics) == 1);
  diagnostic = ms_diag_list_at(&diagnostics, 0);
  TEST_ASSERT(diagnostic != NULL);
  TEST_ASSERT(strcmp(diagnostic->phase, "lex") == 0);
  TEST_ASSERT(strcmp(diagnostic->code, "MS1002") == 0);
  TEST_ASSERT(strcmp(diagnostic->message, "unexpected character") == 0);
  TEST_ASSERT(diagnostic->span.line == 1);
  TEST_ASSERT(diagnostic->span.column == 9);
  TEST_ASSERT(diagnostic->span.length == 1);

  ms_diag_list_destroy(&diagnostics);
  ms_buffer_destroy(&token_bytes);
  return 0;
}

int main(void) {
  TEST_ASSERT(test_tracks_spans_and_newlines() == 0);
  TEST_ASSERT(test_recognizes_basic_punctuation() == 0);
  TEST_ASSERT(test_reports_invalid_character_and_recovers() == 0);
  return 0;
}