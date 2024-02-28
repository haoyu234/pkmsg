// generated by the columns. DO NOT EDIT!

#include "messages.h"

#define USE_COLUMN_MACROS
#include <columns.h>

#ifdef __cplusplus
extern "C" {
#endif

// messages.h:3:8
static const clColumn stUseItemReqColumns[] = {
    FIELD_FIXED_ARRAY(struct stUseItemReq, name),
    FIELD_NUMBER(struct stUseItemReq, itemID),
};
const clColumn stUseItemReqObject[] = {
    DEFINE_OBJECT(struct stUseItemReq, stUseItemReqColumns),
};

// messages.h:8:8
static const clColumn stDropColumns[] = {
    FIELD_NUMBER(struct stDrop, itemID),
    FIELD_NUMBER(struct stDrop, itemNum),
};
const clColumn stDropObject[] = {
    DEFINE_OBJECT(struct stDrop, stDropColumns),
};

// messages.h:13:8
static const clColumn stUseItemRspColumns[] = {
    FIELD_NUMBER(struct stUseItemRsp, code),
    FIELD_NUMBER(struct stUseItemRsp, num),
    FIELD_OBJECT_FLEXIBLE_ARRAY(struct stUseItemRsp, drops, stDropObject),
};
const clColumn stUseItemRspObject[] = {
    DEFINE_OBJECT(struct stUseItemRsp, stUseItemRspColumns),
};

// messages.h:20:7
static const clColumn stValueColumns[] = {
    FIELD_NUMBER(union stValue, i32),
    FIELD_NUMBER(union stValue, u32),
    FIELD_NUMBER(union stValue, c),
    FIELD_NUMBER(union stValue, u8),
    FIELD_FIXED_ARRAY(union stValue, other),
};
// messages.h:30:8
static const clColumn stFuzzColumns[] = {
    FIELD_FIXED_ARRAY(struct stFuzz, name),
    FIELD_NUMBER(struct stFuzz, tag),
    FIELD_UNION(struct stFuzz, v, stValueColumns),
};
const clColumn stFuzzObject[] = {
    DEFINE_OBJECT(struct stFuzz, stFuzzColumns),
};

// messages.h:36:8
static const clColumn stTestsColumns[] = {
    FIELD_NUMBER(struct stTests, epoch),
    FIELD_NUMBER(struct stTests, len),
    FIELD_FLEXIBLE_ARRAY(struct stTests, name),
    FIELD_NUMBER(struct stTests, fuzzNum),
    FIELD_OBJECT_FLEXIBLE_ARRAY(struct stTests, fuzz, stFuzzObject),
};
const clColumn stTestsObject[] = {
    DEFINE_OBJECT(struct stTests, stTestsColumns),
};
#ifdef __cplusplus
}
#endif
