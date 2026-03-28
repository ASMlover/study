#include <string.h>

#include "ms/object.h"
#include "ms/string.h"
#include "ms/value.h"

#include "test_assert.h"

static int expect_formatted_value(MsValue value, const char *expected) {
  char buffer[64];

  TEST_ASSERT(ms_value_format(value, buffer, sizeof(buffer)));
  TEST_ASSERT(strcmp(buffer, expected) == 0);
  return 0;
}

int main(void) {
  MsValue nil_value;
  MsValue bool_value;
  MsValue number_value;
  MsString *empty_string;
  MsString *hello_left;
  MsString *hello_right;
  MsValue empty_string_value;
  MsValue hello_left_value;
  MsValue hello_right_value;
  int boolean = 0;
  double number = 0.0;
  MsString *string_value = NULL;
  MsObject first_object;
  MsObject second_object;

  nil_value = ms_value_nil();
  bool_value = ms_value_bool(7);
  number_value = ms_value_number(42.5);
  empty_string = ms_string_from_cstr("");
  hello_left = ms_string_from_cstr("hello");
  hello_right = ms_string_from_cstr("hello");

  TEST_ASSERT(empty_string != NULL);
  TEST_ASSERT(hello_left != NULL);
  TEST_ASSERT(hello_right != NULL);

  empty_string_value = ms_value_object((MsObject *) empty_string);
  hello_left_value = ms_value_object((MsObject *) hello_left);
  hello_right_value = ms_value_object((MsObject *) hello_right);

  TEST_ASSERT(ms_value_is_nil(nil_value));
  TEST_ASSERT(ms_value_is_bool(bool_value));
  TEST_ASSERT(ms_value_is_number(number_value));
  TEST_ASSERT(ms_value_is_object(hello_left_value));
  TEST_ASSERT(ms_value_is_string(hello_left_value));

  TEST_ASSERT(ms_value_get_bool(bool_value, &boolean));
  TEST_ASSERT(boolean == 1);
  TEST_ASSERT(!ms_value_get_bool(number_value, &boolean));

  TEST_ASSERT(ms_value_get_number(number_value, &number));
  TEST_ASSERT(number == 42.5);
  TEST_ASSERT(!ms_value_get_number(bool_value, &number));

  TEST_ASSERT(ms_value_get_string(hello_left_value, &string_value));
  TEST_ASSERT(string_value == hello_left);
  TEST_ASSERT(!ms_value_get_string(number_value, &string_value));

  TEST_ASSERT(ms_value_is_falsey(nil_value));
  TEST_ASSERT(ms_value_is_falsey(ms_value_bool(0)));
  TEST_ASSERT(ms_value_is_falsey(ms_value_number(0.0)));
  TEST_ASSERT(ms_value_is_falsey(empty_string_value));
  TEST_ASSERT(!ms_value_is_falsey(bool_value));
  TEST_ASSERT(!ms_value_is_falsey(number_value));
  TEST_ASSERT(!ms_value_is_falsey(hello_left_value));

  TEST_ASSERT(ms_value_equals(ms_value_nil(), nil_value));
  TEST_ASSERT(ms_value_equals(ms_value_bool(1), bool_value));
  TEST_ASSERT(!ms_value_equals(ms_value_bool(0), bool_value));
  TEST_ASSERT(ms_value_equals(ms_value_number(42.5), number_value));
  TEST_ASSERT(!ms_value_equals(ms_value_number(13.0), number_value));
  TEST_ASSERT(ms_value_equals(hello_left_value, hello_right_value));
  TEST_ASSERT(!ms_value_equals(hello_left_value, number_value));

  ms_object_init(&first_object, MS_OBJ_FUNCTION);
  ms_object_init(&second_object, MS_OBJ_FUNCTION);
  TEST_ASSERT(ms_value_equals(ms_value_object(&first_object),
                              ms_value_object(&first_object)));
  TEST_ASSERT(!ms_value_equals(ms_value_object(&first_object),
                               ms_value_object(&second_object)));

  TEST_ASSERT(expect_formatted_value(nil_value, "nil") == 0);
  TEST_ASSERT(expect_formatted_value(bool_value, "true") == 0);
  TEST_ASSERT(expect_formatted_value(number_value, "42.5") == 0);
  TEST_ASSERT(expect_formatted_value(hello_left_value, "hello") == 0);

  ms_string_free(empty_string);
  ms_string_free(hello_left);
  ms_string_free(hello_right);
  return 0;
}