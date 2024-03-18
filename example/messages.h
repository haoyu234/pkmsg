#include <stdint.h> // IWYU pragma: keep

struct stUseItemReq {
  char name[32];
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

struct stFuzz {
  char name[32];
  int tag;
  union {
    int32_t i32;  // 1
    uint32_t u32; // 2

    char c;     // 3
    uint8_t u8; // 4

    uint64_t other[2]; // 5

    char name[32]; // 6
  } v;
};

struct stTests {
  uint32_t epoch;

  char name[32];

  uint32_t fuzzNum;
  struct stFuzz fuzz[20];
};
