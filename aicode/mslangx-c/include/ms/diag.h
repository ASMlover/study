#ifndef MS_DIAG_H_
#define MS_DIAG_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct MsSpan {
  const char *file;
  int line;
  int column;
  int length;
} MsSpan;

typedef struct MsDiagnostic {
  const char *phase;
  const char *code;
  const char *message;
  MsSpan span;
} MsDiagnostic;

typedef struct MsDiagnosticList {
  MsDiagnostic *items;
  size_t count;
  size_t capacity;
} MsDiagnosticList;

void ms_diag_list_init(MsDiagnosticList *list);
bool ms_diag_list_append(MsDiagnosticList *list, const MsDiagnostic *diagnostic);
const MsDiagnostic *ms_diag_list_at(const MsDiagnosticList *list, size_t index);
size_t ms_diag_list_count(const MsDiagnosticList *list);
void ms_diag_list_clear(MsDiagnosticList *list);
void ms_diag_list_destroy(MsDiagnosticList *list);

#endif