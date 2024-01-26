#include <assert.h>
#include <columns.h>
#include <pkmsg.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "messages_def.h"

void encode_and_decode(const struct clColumn *column, // 使用工具生成的反射信息
                       void *object,                  // 结构体地址
                       size_t size                    // 结构体大小
) {

  uint8_t serialized_buf[40960];

  // 编码
  const size_t wpos =
      cToBuf(column, object, size, serialized_buf, sizeof(serialized_buf));

  // 重置 c结构体
  memset(object, 0, size);

  // 解码
  const size_t rpos = cFromBuf(column, object, size, serialized_buf, wpos);

  assert(wpos);
  assert(rpos);
  assert(wpos == rpos);
}

void encode_object() {
  struct stUseItemReq use_item_req = {.itemID = 1001};

  encode_and_decode(stUseItemReqObject, &use_item_req, sizeof(use_item_req));

  assert(use_item_req.itemID == 1001);
}

void encode_dyn_array() {
  const uint32_t num = rand() % 10;

  struct stUseItemRsp use_item_rsp;
  use_item_rsp.code = 200;
  use_item_rsp.num = num;

  for (int i = 0; i < num; ++i) {
    use_item_rsp.drops[i].itemID = i + 1;
    use_item_rsp.drops[i].itemNum = i + 1;
  }

  encode_and_decode(stUseItemRspObject, &use_item_rsp, sizeof(use_item_rsp));

  assert(num == use_item_rsp.num);

  for (int i = 0; i < num; ++i) {
    assert(use_item_rsp.drops[i].itemID == i + 1);
    assert(use_item_rsp.drops[i].itemNum == i + 1);
  }
}

int main(int argc, char *args[]) {
  int num = 10000000;
  if (argc > 1) {
    num = atoi(args[1]);
  }
  for (int i = 0; i < num; ++i) {
    encode_object();
    encode_dyn_array();
  }
  return 0;
}