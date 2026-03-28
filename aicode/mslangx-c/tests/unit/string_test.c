#include <string.h>

#include "ms/string.h"

#include "test_assert.h"

int main(void) {
  char bytes[] = "module";
  MsString *left;
  MsString *right;
  MsString *empty;

  left = ms_string_new(bytes, strlen(bytes));
  right = ms_string_from_cstr("module");
  empty = ms_string_from_cstr("");

  TEST_ASSERT(left != NULL);
  TEST_ASSERT(right != NULL);
  TEST_ASSERT(empty != NULL);

  bytes[0] = 'X';

  TEST_ASSERT(left->object.type == MS_OBJ_STRING);
  TEST_ASSERT(left->object.marked == 0);
  TEST_ASSERT(left->object.next == NULL);
  TEST_ASSERT(left->length == 6);
  TEST_ASSERT(left->hash == ms_string_hash_bytes("module", 6));
  TEST_ASSERT(strcmp(left->bytes, "module") == 0);
  TEST_ASSERT(left->bytes[left->length] == '\0');

  TEST_ASSERT(right->object.type == MS_OBJ_STRING);
  TEST_ASSERT(right->length == 6);
  TEST_ASSERT(strcmp(right->bytes, "module") == 0);

  TEST_ASSERT(left != right);
  TEST_ASSERT(ms_string_equals(left, right));
  TEST_ASSERT(!ms_string_equals(left, empty));

  TEST_ASSERT(empty->length == 0);
  TEST_ASSERT(strcmp(empty->bytes, "") == 0);
  TEST_ASSERT(empty->hash == ms_string_hash_bytes("", 0));

  ms_string_free(left);
  ms_string_free(right);
  ms_string_free(empty);
  return 0;
}