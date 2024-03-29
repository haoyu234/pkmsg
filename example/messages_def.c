// generated by the columns. DO NOT EDIT!

#define USE_COLUMN_MACROS
#include <columns.h>

#include "messages.h"

#ifdef __cplusplus
extern "C" {
#endif

// messages.h:12:8
static const clColumn c__S_stUseItemReq[] = {
    DEFINE_FIELD_STRING(struct stUseItemReq, name),
    DEFINE_FIELD_NUMBER(struct stUseItemReq, itemID),
};
const clColumn stUseItemReqObject[] = {
    DEFINE_OBJECT(struct stUseItemReq, c__S_stUseItemReq),
};
// messages.h:17:8
static const clColumn c__S_stDrop[] = {
    DEFINE_FIELD_NUMBER(struct stDrop, itemID),
    DEFINE_FIELD_NUMBER(struct stDrop, itemNum),
};
const clColumn stDropObject[] = {
    DEFINE_OBJECT(struct stDrop, c__S_stDrop),
};
// messages.h:23:8
static const clColumn c__S_stUseItemRsp[] = {
    DEFINE_FIELD_NUMBER(struct stUseItemRsp, code),
    DEFINE_FIELD_NUMBER(struct stUseItemRsp, num),
    DEFINE_FIELD_OBJECT_FLEXIBLE_ARRAY(struct stUseItemRsp, drops, stDropObject),
};
const clColumn stUseItemRspObject[] = {
    DEFINE_OBJECT(struct stUseItemRsp, c__S_stUseItemRsp),
};
union c__S_stFuzz_U_messages_h_424 {
  int32_t i32;  // 1
  uint32_t u32; // 2

  char c;     // 3
  uint8_t u8; // 4

  uint64_t other[2]; // 5

  char name[32]; // 6
};
// messages.h:34:3
static const clColumn c__S_stFuzz_U_messages_h_424[] = {
    DEFINE_FIELD_NUMBER(union c__S_stFuzz_U_messages_h_424, i32),
    DEFINE_FIELD_NUMBER(union c__S_stFuzz_U_messages_h_424, u32),
    DEFINE_FIELD_NUMBER(union c__S_stFuzz_U_messages_h_424, c),
    DEFINE_FIELD_NUMBER(union c__S_stFuzz_U_messages_h_424, u8),
    DEFINE_FIELD_FIXED_ARRAY(union c__S_stFuzz_U_messages_h_424, other),
    DEFINE_FIELD_STRING(union c__S_stFuzz_U_messages_h_424, name),
};
// messages.h:31:8
static const clColumn c__S_stFuzz[] = {
    DEFINE_FIELD_STRING(struct stFuzz, name),
    DEFINE_FIELD_NUMBER(struct stFuzz, tag),
    DEFINE_FIELD_UNION(struct stFuzz, v, c__S_stFuzz_U_messages_h_424),
};
const clColumn stFuzzObject[] = {
    DEFINE_OBJECT(struct stFuzz, c__S_stFuzz),
};
// messages.h:47:8
static const clColumn c__S_stTests[] = {
    DEFINE_FIELD_NUMBER(struct stTests, epoch),
    DEFINE_FIELD_STRING(struct stTests, name),
    DEFINE_FIELD_NUMBER(struct stTests, fuzzNum),
    DEFINE_FIELD_OBJECT_FLEXIBLE_ARRAY(struct stTests, fuzz, stFuzzObject),
};
const clColumn stTestsObject[] = {
    DEFINE_OBJECT(struct stTests, c__S_stTests),
};

/* plugin output */
const struct clColumn* get_message_def(int cmd)
{
  switch (cmd)
  {
  case USE_ITEM_REQ: return stUseItemReqObject;
  case USE_ITEM_RSP: return stUseItemRspObject;
  case FUZZ_STRUCT: return stFuzzObject;
  default: return NULL;
  }
}

#ifdef __cplusplus
}
#endif
