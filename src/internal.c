#include <assert.h>
#include <columns.h>

#include "internal.h"

uint32_t ReadMemoryAsUInt32(uint8_t kind, const void *addr)
{
    struct StorageUnion storage;
    switch (kind)
    {
        case cl_COLUMN_INT8:
            return UNSAFE_READ_MEMORY(addr, int8_t, storage) > 0 ? storage.i8 : 0;
        case cl_COLUMN_INT16:
            return UNSAFE_READ_MEMORY(addr, int16_t, storage) > 0 ? storage.i16 : 0;
        case cl_COLUMN_INT32:
            return UNSAFE_READ_MEMORY(addr, int32_t, storage) > 0 ? storage.i32 : 0;
        case cl_COLUMN_INT64:
            return UNSAFE_READ_MEMORY(addr, int64_t, storage) > 0 ? storage.i64 : 0;
        case cl_COLUMN_UINT8:
            return UNSAFE_READ_MEMORY(addr, uint8_t, storage);
        case cl_COLUMN_UINT16:
            return UNSAFE_READ_MEMORY(addr, uint16_t, storage);
        case cl_COLUMN_UINT32:
            return UNSAFE_READ_MEMORY(addr, uint32_t, storage);
        case cl_COLUMN_UINT64:
            return UNSAFE_READ_MEMORY(addr, uint64_t, storage);
        default:
            assert(false);
    }
    return 0;
}
