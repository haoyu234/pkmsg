#include <inttypes.h>

union stValue {
  int32_t i32;
  uint32_t u32;

  char c;
  uint8_t u8;

  uint64_t other[2];
};

struct stFuzz {
  char name[32];
  int tag;
  union stValue v;
};

struct stTests {
  uint32_t epoch;

  uint32_t len;
  char name[32];

  uint32_t fuzzNum;
  struct stFuzz fuzz[20];
};