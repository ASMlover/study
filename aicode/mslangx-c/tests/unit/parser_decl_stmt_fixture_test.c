#define _CRT_SECURE_NO_WARNINGS

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms/arena.h"
#include "ms/ast.h"
#include "ms/buffer.h"
#include "ms/diag.h"
#include "ms/parser.h"

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
  MsArena arena;
  MsDiagnosticList diagnostics;
  MsBuffer ast_snapshot;
  MsBuffer diag_snapshot;
  char expected_ast_path[512];
  char expected_diag_path[512];
  char *source;
  MsAstNode *root;
  size_t i;

  if (argc != 2) {
    fprintf(stderr, "usage: parser_decl_stmt.fixture <fixture.ms>\n");
    return 1;
  }

  source = read_file(argv[1], NULL);
  if (source == NULL) {
    return 1;
  }

  ms_arena_init(&arena, 4096);
  ms_diag_list_init(&diagnostics);
  ms_buffer_init(&ast_snapshot);
  ms_buffer_init(&diag_snapshot);

  root = ms_parse_source(argv[1], source, &arena, &diagnostics);
  if (root != NULL && !ms_ast_dump(&ast_snapshot, root)) {
    free(source);
    ms_buffer_destroy(&diag_snapshot);
    ms_buffer_destroy(&ast_snapshot);
    ms_diag_list_destroy(&diagnostics);
    ms_arena_destroy(&arena);
    return 1;
  }

  for (i = 0; i < ms_diag_list_count(&diagnostics); ++i) {
    const MsDiagnostic *diagnostic = ms_diag_list_at(&diagnostics, i);

    if (diagnostic == NULL || !append_diag_snapshot(&diag_snapshot, diagnostic)) {
      free(source);
      ms_buffer_destroy(&diag_snapshot);
      ms_buffer_destroy(&ast_snapshot);
      ms_diag_list_destroy(&diagnostics);
      ms_arena_destroy(&arena);
      return 1;
    }
  }

  snprintf(expected_ast_path, sizeof(expected_ast_path), "%s.ast", argv[1]);
  snprintf(expected_diag_path, sizeof(expected_diag_path), "%s.diag", argv[1]);

  if (!compare_snapshot("ast", expected_ast_path, &ast_snapshot) ||
      !compare_snapshot("diagnostics", expected_diag_path, &diag_snapshot)) {
    free(source);
    ms_buffer_destroy(&diag_snapshot);
    ms_buffer_destroy(&ast_snapshot);
    ms_diag_list_destroy(&diagnostics);
    ms_arena_destroy(&arena);
    return 1;
  }

  free(source);
  ms_buffer_destroy(&diag_snapshot);
  ms_buffer_destroy(&ast_snapshot);
  ms_diag_list_destroy(&diagnostics);
  ms_arena_destroy(&arena);
  return 0;
}