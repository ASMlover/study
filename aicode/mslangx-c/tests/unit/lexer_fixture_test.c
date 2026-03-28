#define _CRT_SECURE_NO_WARNINGS

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms/buffer.h"
#include "ms/diag.h"
#include "ms/lexer.h"
#include "ms/token.h"

static int append_string(MsBuffer *buffer, const char *text) {
  return ms_buffer_append(buffer, text, strlen(text));
}

static int append_char(MsBuffer *buffer, char ch) {
  return ms_buffer_append(buffer, &ch, 1);
}

static int append_escaped_slice(MsBuffer *buffer, const char *text, size_t length) {
  size_t i;

  for (i = 0; i < length; ++i) {
    switch (text[i]) {
      case '\\':
        if (!append_string(buffer, "\\\\")) {
          return 0;
        }
        break;
      case '"':
        if (!append_string(buffer, "\\\"")) {
          return 0;
        }
        break;
      case '\n':
        if (!append_string(buffer, "\\n")) {
          return 0;
        }
        break;
      case '\t':
        if (!append_string(buffer, "\\t")) {
          return 0;
        }
        break;
      default:
        if (!append_char(buffer, text[i])) {
          return 0;
        }
        break;
    }
  }

  return 1;
}

static int append_token_snapshot(MsBuffer *buffer, MsToken token) {
  char header[128];
  int written;

  written = snprintf(header,
                     sizeof(header),
                     "%d:%d-%d %s ",
                     token.line,
                     token.column,
                     token.end_column,
                     ms_token_kind_name(token.kind));
  if (written < 0 || (size_t) written >= sizeof(header)) {
    return 0;
  }
  if (!ms_buffer_append(buffer, header, (size_t) written)) {
    return 0;
  }
  if (!append_char(buffer, '"')) {
    return 0;
  }
  if (!append_escaped_slice(buffer, token.start, token.length)) {
    return 0;
  }
  if (!append_string(buffer, "\"\n")) {
    return 0;
  }

  return 1;
}

static int append_diag_snapshot(MsBuffer *buffer, const MsDiagnostic *diagnostic) {
  char line[256];
  int written;

  written = snprintf(line,
                     sizeof(line),
                     "%s %s %d:%d+%d %s\n",
                     diagnostic->phase,
                     diagnostic->code,
                     diagnostic->span.line,
                     diagnostic->span.column,
                     diagnostic->span.length,
                     diagnostic->message);
  if (written < 0 || (size_t) written >= sizeof(line)) {
    return 0;
  }

  return ms_buffer_append(buffer, line, (size_t) written);
}

static char *read_file(const char *path, size_t *out_length) {
  FILE *stream;
  long size;
  char *data;
  size_t read_size;

  stream = fopen(path, "rb");
  if (stream == NULL) {
    fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
    return NULL;
  }

  if (fseek(stream, 0, SEEK_END) != 0) {
    fclose(stream);
    return NULL;
  }
  size = ftell(stream);
  if (size < 0) {
    fclose(stream);
    return NULL;
  }
  if (fseek(stream, 0, SEEK_SET) != 0) {
    fclose(stream);
    return NULL;
  }

  data = (char *) malloc((size_t) size + 1);
  if (data == NULL) {
    fclose(stream);
    return NULL;
  }

  read_size = fread(data, 1, (size_t) size, stream);
  fclose(stream);
  if (read_size != (size_t) size) {
    free(data);
    return NULL;
  }

  data[size] = '\0';
  if (out_length != NULL) {
    *out_length = (size_t) size;
  }
  return data;
}

static int compare_snapshot(const char *label,
                            const char *expected_path,
                            const MsBuffer *actual) {
  size_t expected_length = 0;
  char *expected = read_file(expected_path, &expected_length);

  if (expected == NULL) {
    return 0;
  }

  if (expected_length != actual->length ||
      memcmp(expected, actual->data, actual->length) != 0) {
    fprintf(stderr, "%s snapshot mismatch: %s\n", label, expected_path);
    fprintf(stderr, "expected:\n%.*s", (int) expected_length, expected);
    fprintf(stderr, "actual:\n%.*s", (int) actual->length, actual->data);
    free(expected);
    return 0;
  }

  free(expected);
  return 1;
}

int main(int argc, char **argv) {
  MsLexer lexer;
  MsDiagnosticList diagnostics;
  MsBuffer token_snapshot;
  MsBuffer diag_snapshot;
  char expected_tokens_path[512];
  char expected_diag_path[512];
  char *source;
  size_t i;

  if (argc != 2) {
    fprintf(stderr, "usage: lexer.fixture <fixture.ms>\n");
    return 1;
  }

  source = read_file(argv[1], NULL);
  if (source == NULL) {
    return 1;
  }

  ms_diag_list_init(&diagnostics);
  ms_buffer_init(&token_snapshot);
  ms_buffer_init(&diag_snapshot);
  ms_lexer_init(&lexer, argv[1], source, &diagnostics);

  for (;;) {
    MsToken token = ms_lexer_next(&lexer);

    if (!append_token_snapshot(&token_snapshot, token)) {
      free(source);
      ms_diag_list_destroy(&diagnostics);
      ms_buffer_destroy(&token_snapshot);
      ms_buffer_destroy(&diag_snapshot);
      return 1;
    }
    if (token.kind == MS_TOKEN_EOF) {
      break;
    }
  }

  for (i = 0; i < ms_diag_list_count(&diagnostics); ++i) {
    const MsDiagnostic *diagnostic = ms_diag_list_at(&diagnostics, i);

    if (diagnostic == NULL || !append_diag_snapshot(&diag_snapshot, diagnostic)) {
      free(source);
      ms_diag_list_destroy(&diagnostics);
      ms_buffer_destroy(&token_snapshot);
      ms_buffer_destroy(&diag_snapshot);
      return 1;
    }
  }

  snprintf(expected_tokens_path, sizeof(expected_tokens_path), "%s.tokens", argv[1]);
  snprintf(expected_diag_path, sizeof(expected_diag_path), "%s.diag", argv[1]);

  if (!compare_snapshot("tokens", expected_tokens_path, &token_snapshot) ||
      !compare_snapshot("diagnostics", expected_diag_path, &diag_snapshot)) {
    free(source);
    ms_diag_list_destroy(&diagnostics);
    ms_buffer_destroy(&token_snapshot);
    ms_buffer_destroy(&diag_snapshot);
    return 1;
  }

  free(source);
  ms_diag_list_destroy(&diagnostics);
  ms_buffer_destroy(&token_snapshot);
  ms_buffer_destroy(&diag_snapshot);
  return 0;
}