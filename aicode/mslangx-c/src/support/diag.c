#include "ms/diag.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *ms_diag_strdup(const char *text) {
  size_t size;
  char *copy;

  if (!text) {
    return NULL;
  }

  size = strlen(text) + 1;
  copy = malloc(size);
  if (!copy) {
    return NULL;
  }

  memcpy(copy, text, size);
  return copy;
}

static void ms_diag_release(MsDiagnostic *diagnostic) {
  free((char *)diagnostic->phase);
  free((char *)diagnostic->code);
  free((char *)diagnostic->message);
  free((char *)diagnostic->span.file);
}

static bool ms_diag_list_grow(MsDiagnosticList *list) {
  size_t new_capacity;
  MsDiagnostic *items;

  if (list->capacity >= (SIZE_MAX / 2)) {
    return false;
  }

  new_capacity = list->capacity ? list->capacity * 2 : 4;
  items = realloc(list->items, new_capacity * sizeof(*items));
  if (!items) {
    return false;
  }

  list->items = items;
  list->capacity = new_capacity;
  return true;
}

void ms_diag_list_init(MsDiagnosticList *list) {
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

bool ms_diag_list_append(MsDiagnosticList *list,
                         const MsDiagnostic *diagnostic) {
  MsDiagnostic copy = {0};

  if (!list || !diagnostic) {
    return false;
  }

  if (list->count == list->capacity && !ms_diag_list_grow(list)) {
    return false;
  }

  copy.span = diagnostic->span;
  copy.phase = ms_diag_strdup(diagnostic->phase);
  copy.code = ms_diag_strdup(diagnostic->code);
  copy.message = ms_diag_strdup(diagnostic->message);
  copy.span.file = ms_diag_strdup(diagnostic->span.file);
  if ((diagnostic->phase && !copy.phase) ||
      (diagnostic->code && !copy.code) ||
      (diagnostic->message && !copy.message) ||
      (diagnostic->span.file && !copy.span.file)) {
    ms_diag_release(&copy);
    return false;
  }

  list->items[list->count++] = copy;
  return true;
}

const MsDiagnostic *ms_diag_list_at(const MsDiagnosticList *list,
                                    size_t index) {
  if (!list || index >= list->count) {
    return NULL;
  }

  return &list->items[index];
}

size_t ms_diag_list_count(const MsDiagnosticList *list) {
  return list ? list->count : 0;
}

void ms_diag_list_clear(MsDiagnosticList *list) {
  size_t i;

  if (!list) {
    return;
  }

  for (i = 0; i < list->count; ++i) {
    ms_diag_release(&list->items[i]);
  }

  list->count = 0;
}

void ms_diag_list_destroy(MsDiagnosticList *list) {
  if (!list) {
    return;
  }

  ms_diag_list_clear(list);
  free(list->items);
  list->items = NULL;
  list->capacity = 0;
}