#include <inttypes.h>

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
