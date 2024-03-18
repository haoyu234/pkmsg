// generated by the columns. DO NOT EDIT!

#include "messages.h"

#define USE_COLUMN_MACROS
#include <columns.h>

#ifdef __cplusplus
extern "C" {
#endif

// messages.h:3:8
static const clColumn c__S_stUseItemReq[] = {
    FIELD_STRING(struct stUseItemReq, name),
    FIELD_NUMBER(struct stUseItemReq, itemID),
};
const clColumn stUseItemReqObject[] = {
    DEFINE_OBJECT(struct stUseItemReq, c__S_stUseItemReq),
};
// messages.h:8:8
static const clColumn c__S_stDrop[] = {
    FIELD_NUMBER(struct stDrop, itemID),
    FIELD_NUMBER(struct stDrop, itemNum),
};
const clColumn stDropObject[] = {
    DEFINE_OBJECT(struct stDrop, c__S_stDrop),
};
// messages.h:13:8
static const clColumn c__S_stUseItemRsp[] = {
    FIELD_NUMBER(struct stUseItemRsp, code),
    FIELD_NUMBER(struct stUseItemRsp, num),
    FIELD_OBJECT_FLEXIBLE_ARRAY(struct stUseItemRsp, drops, stDropObject),
};
const clColumn stUseItemRspObject[] = {
    DEFINE_OBJECT(struct stUseItemRsp, c__S_stUseItemRsp),
};
// messages.h:23:3
static const clColumn c__S_stFuzz_U_messages_h_296[] = {
    FIELD_NUMBER(__typeof(((struct stFuzz *)NULL)->v), i32),
    FIELD_NUMBER(__typeof(((struct stFuzz *)NULL)->v), u32),
    FIELD_NUMBER(__typeof(((struct stFuzz *)NULL)->v), c),
    FIELD_NUMBER(__typeof(((struct stFuzz *)NULL)->v), u8),
    FIELD_FIXED_ARRAY(__typeof(((struct stFuzz *)NULL)->v), other),
    FIELD_STRING(__typeof(((struct stFuzz *)NULL)->v), name),
};
// messages.h:20:8
static const clColumn c__S_stFuzz[] = {
    FIELD_STRING(struct stFuzz, name),
    FIELD_NUMBER(struct stFuzz, tag),
    FIELD_UNION(struct stFuzz, v, c__S_stFuzz_U_messages_h_296),
};
const clColumn stFuzzObject[] = {
    DEFINE_OBJECT(struct stFuzz, c__S_stFuzz),
};
// messages.h:36:8
static const clColumn c__S_stTests[] = {
    FIELD_NUMBER(struct stTests, epoch),
    FIELD_STRING(struct stTests, name),
    FIELD_NUMBER(struct stTests, fuzzNum),
    FIELD_OBJECT_FLEXIBLE_ARRAY(struct stTests, fuzz, stFuzzObject),
};
const clColumn stTestsObject[] = {
    DEFINE_OBJECT(struct stTests, c__S_stTests),
};
#ifdef __cplusplus
}
#endif
