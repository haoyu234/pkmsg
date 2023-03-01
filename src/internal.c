#include <assert.h>
#include <columns.h>

#include "internal.h"

uint32_t ReadMemoryAsUInt32(uint8_t kind, const void *addr)
{
    switch (kind)
    {
        case cl_COLUMN_INT8:
            return DEREF(addr, int8_t) > 0 ?
                DEREF(addr, int8_t):
                0;
        case cl_COLUMN_INT16:
            return DEREF(addr, int16_t) > 0 ?
                DEREF(addr, int16_t):
                0;
        case cl_COLUMN_INT32:
            return DEREF(addr, int32_t) > 0 ?
                DEREF(addr, int32_t):
                0;
        case cl_COLUMN_INT64:
            return DEREF(addr, int64_t) > 0 ?
                DEREF(addr, int64_t):
                0;
        case cl_COLUMN_UINT8:
            return DEREF(addr, uint8_t);
        case cl_COLUMN_UINT16:
            return DEREF(addr, uint16_t);
        case cl_COLUMN_UINT32:
            return DEREF(addr, uint32_t);
        case cl_COLUMN_UINT64:
            return DEREF(addr, uint64_t);
        default:
            assert(false);
    }
    return 0;
}
