#include <assert.h>
#include <columns.h>
#include <pkmsg.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "messages_def.h"

void encode_and_decode(const struct clColumn *column, void *object,
                       size_t size) {
  uint8_t buf[40960];

  // encode
  const size_t size1 = cToBuf(column, object, size, buf, sizeof(buf));

  // clear
  memset(object, 0, size);

  // decode
  const size_t size2 = cFromBuf(column, object, size, buf, size1);

  assert(size1);
  assert(size2);
  assert(size1 == size2);
}

void encode_object() {
  struct stUseItemReq useItemReq = {.itemID = 1001};

  encode_and_decode(stUseItemReqObject, &useItemReq, sizeof(useItemReq));

  assert(useItemReq.itemID == 1001);
}

void encode_dyn_array() {
  const uint32_t num = rand() % 10;

  struct stUseItemRsp useItemRsp;
  useItemRsp.code = 200;
  useItemRsp.num = num;

  for (int i = 0; i < num; ++i) {
    useItemRsp.drops[i].itemID = i + 1;
    useItemRsp.drops[i].itemNum = i + 1;
  }

  encode_and_decode(stUseItemRspObject, &useItemRsp, sizeof(useItemRsp));

  assert(num == useItemRsp.num);

  for (int i = 0; i < num; ++i) {
    assert(useItemRsp.drops[i].itemID == i + 1);
    assert(useItemRsp.drops[i].itemNum == i + 1);
  }
}

int main(int argc, char *args[]) {
  int num = 1;
  if (argc > 1) {
    num = atoi(args[1]);
  }
  for (int i = 0; i < num; ++i) {
    encode_object();
    encode_dyn_array();
  }
  return 0;
}