#include <stdint.h>
#include <string.h>

#include "ms/buffer.h"

#include "test_assert.h"

int main(void) {
  MsBuffer buffer;
  const char hello[] = "hello";
  const char world[] = "world";
  const char patch[] = "XYZ";

  ms_buffer_init(&buffer);

  TEST_ASSERT(ms_buffer_append(&buffer, hello, sizeof(hello) - 1));
  TEST_ASSERT(ms_buffer_append(&buffer, world, sizeof(world) - 1));
  TEST_ASSERT(buffer.length == 10);
  TEST_ASSERT(buffer.capacity >= buffer.length);
  TEST_ASSERT(memcmp(buffer.data, "helloworld", 10) == 0);

  TEST_ASSERT(ms_buffer_write(&buffer, 5, patch, sizeof(patch) - 1));
  TEST_ASSERT(memcmp(buffer.data, "helloXYZld", 10) == 0);
  TEST_ASSERT(buffer.length == 10);
  TEST_ASSERT(buffer.capacity >= buffer.length);

  TEST_ASSERT(!ms_buffer_write(&buffer, buffer.length - 1, patch,
                               sizeof(patch) - 1));
  TEST_ASSERT(!ms_buffer_reserve(&buffer, (size_t)-1));
  TEST_ASSERT(buffer.length == 10);
  TEST_ASSERT(buffer.capacity >= buffer.length);

  ms_buffer_destroy(&buffer);
  return 0;
}