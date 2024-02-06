#include <stdint.h> // IWYU pragma: keep

struct stUseItemReq {
  uint32_t itemID;
};

struct stDrop {
  uint32_t itemID;
  uint32_t itemNum;
};

struct stUseItemRsp {
  uint32_t code;
  uint32_t num;

  struct stDrop drops[10];
};

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
