#include <assert.h>
#include <columns.h>
#include <pkmsg.h>
#include <string.h>

#include "messages.h"
#include "messages_def.h"

#define MY_MIN(a, b) (a) > (b) ? (b) : (a)
#define MY_MAX(a, b) (a) < (b) ? (b) : (a)
#define MY_ARRAY_SIZE(ARRAY) sizeof(ARRAY) / sizeof(ARRAY[0])
#define MY_ADJUST_SIZE(SZ, FIELD) SZ = MY_MIN(SZ, MY_ARRAY_SIZE(FIELD))

void test_1(const uint8_t *data, size_t size) {
  struct stTests object;

  const size_t rpos =
      pk_decode(stTestsObject, &object, sizeof(object), data, size);
  if (rpos > 0) {
    uint8_t serialized_buf[10240];
    const size_t wpos = pk_encode(stTestsObject, &object, sizeof(object),
                                  serialized_buf, sizeof(serialized_buf));

    assert(rpos == wpos);
  }
}

int test_2(const uint8_t *data, size_t size) {
  struct stTests object;

  memcpy(&object, data, MY_MIN(sizeof(object), size));

  object.name[MY_ARRAY_SIZE(object.name) - 1] = 0;

  MY_ADJUST_SIZE(object.fuzzNum, object.fuzz);

  for (int i = 0; i < object.fuzzNum; ++i) {
    object.fuzz[i].tag = MY_MAX(object.fuzz[i].tag, 1);
    object.fuzz[i].tag = MY_MIN(object.fuzz[i].tag, 4);
    object.fuzz[i].name[MY_ARRAY_SIZE(object.fuzz[i].name) - 1] = 0;
  }

  uint8_t serialized_buf[10240];
  const size_t wpos = pk_encode(stTestsObject, &object, sizeof(object),
                                serialized_buf, sizeof(serialized_buf));
  assert(wpos);

  struct stTests object2;
  memset(&object2, 0, sizeof(object2));

  const size_t rpos =
      pk_decode(stTestsObject, &object2, sizeof(object2), serialized_buf, wpos);

  assert(rpos);
  assert(rpos == wpos);
  return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  test_1(data, size);
  test_2(data, size);
  return 0;
}
