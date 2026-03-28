#include <string.h>

#include "ms/diag.h"

#include "test_assert.h"

int main(void) {
  MsDiagnosticList list;
  char phase[] = "parse";
  char code[] = "MS2001";
  char message[] = "expected expression";
  char file[] = "sample.ms";
  MsDiagnostic first;
  MsDiagnostic second;
  const MsDiagnostic *stored_first;
  const MsDiagnostic *stored_second;

  ms_diag_list_init(&list);

  first.phase = phase;
  first.code = code;
  first.message = message;
  first.span.file = file;
  first.span.line = 3;
  first.span.column = 5;
  first.span.length = 2;

  second.phase = "resolve";
  second.code = "MS3001";
  second.message = "undefined variable";
  second.span.file = "sample.ms";
  second.span.line = 8;
  second.span.column = 1;
  second.span.length = 4;

  TEST_ASSERT(ms_diag_list_append(&list, &first));
  TEST_ASSERT(ms_diag_list_append(&list, &second));

  phase[0] = 'x';
  code[2] = '9';
  message[0] = 'X';
  file[0] = 'X';

  TEST_ASSERT(ms_diag_list_count(&list) == 2);

  stored_first = ms_diag_list_at(&list, 0);
  stored_second = ms_diag_list_at(&list, 1);

  TEST_ASSERT(stored_first != NULL);
  TEST_ASSERT(stored_second != NULL);
  TEST_ASSERT(strcmp(stored_first->phase, "parse") == 0);
  TEST_ASSERT(strcmp(stored_first->code, "MS2001") == 0);
  TEST_ASSERT(strcmp(stored_first->message, "expected expression") == 0);
  TEST_ASSERT(strcmp(stored_first->span.file, "sample.ms") == 0);
  TEST_ASSERT(stored_first->span.line == 3);
  TEST_ASSERT(stored_first->span.column == 5);
  TEST_ASSERT(stored_first->span.length == 2);
  TEST_ASSERT(strcmp(stored_second->phase, "resolve") == 0);
  TEST_ASSERT(strcmp(stored_second->code, "MS3001") == 0);

  ms_diag_list_clear(&list);

  TEST_ASSERT(ms_diag_list_count(&list) == 0);
  TEST_ASSERT(ms_diag_list_at(&list, 0) == NULL);

  ms_diag_list_destroy(&list);
  return 0;
}