#include <assert.h>
#include <columns.h>

#include "internal.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

uint32_t read_u32(uint8_t kind, const void *addr) {
  struct storage_union storage;
  switch (kind) {
  case cl_COLUMN_INT8:
    UNSAFE_READ_MEMORY(addr, int8_t, storage);
    return max(storage.i8, 0);
  case cl_COLUMN_INT16:
    UNSAFE_READ_MEMORY(addr, int16_t, storage);
    return max(storage.i16, 0);
  case cl_COLUMN_INT32:
    UNSAFE_READ_MEMORY(addr, int32_t, storage);
    return max(storage.i32, 0);
  case cl_COLUMN_INT64:
    UNSAFE_READ_MEMORY(addr, int64_t, storage);
    return max(storage.i64, 0);
  case cl_COLUMN_UINT8:
    UNSAFE_READ_MEMORY(addr, uint8_t, storage);
    return storage.u8;
  case cl_COLUMN_UINT16:
    UNSAFE_READ_MEMORY(addr, uint16_t, storage);
    return storage.u16;
  case cl_COLUMN_UINT32:
    UNSAFE_READ_MEMORY(addr, uint32_t, storage);
    return storage.u32;
  case cl_COLUMN_UINT64:
    UNSAFE_READ_MEMORY(addr, uint64_t, storage);
    return storage.u64;
  default:
    assert(false);
  }
  return 0;
}

void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    void *context) {
  switch (column->kind) {
  case cl_COLUMN_INT8:
  case cl_COLUMN_INT16:
  case cl_COLUMN_INT32:
  case cl_COLUMN_INT64:
  case cl_COLUMN_INT128:
  case cl_COLUMN_INT256:
  case cl_COLUMN_UINT8:
  case cl_COLUMN_UINT16:
  case cl_COLUMN_UINT32:
  case cl_COLUMN_UINT64:
  case cl_COLUMN_UINT128:
  case cl_COLUMN_UINT256:
  case cl_COLUMN_FLOAT8:
  case cl_COLUMN_FLOAT16:
  case cl_COLUMN_FLOAT32:
  case cl_COLUMN_FLOAT64:
  case cl_COLUMN_FLOAT128:
  case cl_COLUMN_FLOAT256:
  case cl_COLUMN_BOOL:
    return visitor->visit_number(visitor, column, context);

  case cl_COLUMN_OBJECT:
    return visitor->visit_object(visitor, column, context);

  case cl_COLUMN_UNION:
    return visitor->visit_union(visitor, column, context);

  case cl_COLUMN_FIXED_ARRAY:
    return visitor->visit_fixed_array(visitor, column, context);

  case cl_COLUMN_FLEXIBLE_ARRAY:
    return visitor->visit_flexible_array(visitor, column, context);

  default:
    assert(false);
  }
}
