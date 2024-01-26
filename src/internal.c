#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "internal.h"

const int SIZES[] = {
    0,        // cl_COLUMN_NONE
    1,        // cl_COLUMN_INT8
    2,        // cl_COLUMN_INT16
    4,        // cl_COLUMN_INT32
    8,        // cl_COLUMN_INT64
    16,       // cl_COLUMN_INT128
    32,       // cl_COLUMN_INT256
    1,        // cl_COLUMN_UINT8
    2,        // cl_COLUMN_UINT16
    4,        // cl_COLUMN_UINT32
    8,        // cl_COLUMN_UINT64
    16,       // cl_COLUMN_UINT128
    32,       // cl_COLUMN_UINT256
    1,        // cl_COLUMN_FLOAT8
    2,        // cl_COLUMN_FLOAT16
    4,        // cl_COLUMN_FLOAT32
    8,        // cl_COLUMN_FLOAT64
    16,       // cl_COLUMN_FLOAT128
    32,       // cl_COLUMN_FLOAT256
    [50] = 1, // cl_COLUMN_BOOL
};

#define max(a, b) ((a) > (b) ? (a) : (b))

#define UNSAFE_READ_MEMORY(ptr, type, storage)                                 \
  do {                                                                         \
    memcpy(&storage, ptr, sizeof(type));                                       \
  } while (false)

#define UNSAFE_WRITE_MEMORY(ptr, type, storage)                                \
  do {                                                                         \
    memcpy(ptr, &storage, sizeof(type));                                       \
  } while (false)

struct full_storage {
  union {
    bool b;

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;

    float f;
    double d;

#if __HAVE_FLOAT16
    _Float16 f16;
#endif
#if __HAVE_FLOAT32
    _Float32 f32;
#endif
#if __HAVE_FLOAT64
    _Float64 f64;
#endif
#if __HAVE_FLOAT128
    _Float128 f128;
#endif
  };
};

#define CHECK_MEMORY_BOUNDARY(ctx, kind, addr)                                 \
  do {                                                                         \
    if ((uint8_t *)(addr) + SIZE((kind)) > (ctx)->end_addr) {                  \
      (ctx)->has_error = true;                                                 \
      return;                                                                  \
    }                                                                          \
  } while (false)

void pack_addr(struct context *context, uint8_t kind, const void *addr) {
  CHECK_MEMORY_BOUNDARY(context, kind, addr);

  struct full_storage storage;
  switch (kind) {
  case cl_COLUMN_BOOL:
    CHECK_COND_ERROR(context, cmp_write_bool(&context->ctx, *(bool *)addr));
    break;
  case cl_COLUMN_INT8:
    CHECK_COND_ERROR(context, cmp_write_s8(&context->ctx, *(int8_t *)addr));
    break;
  case cl_COLUMN_INT16:
    UNSAFE_READ_MEMORY(addr, int16_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s16(&context->ctx, storage.i16));
    break;
  case cl_COLUMN_INT32:
    UNSAFE_READ_MEMORY(addr, int32_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s32(&context->ctx, storage.i32));
    break;
  case cl_COLUMN_INT64:
    UNSAFE_READ_MEMORY(addr, int64_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s64(&context->ctx, storage.i64));
    break;
  case cl_COLUMN_UINT8:
    CHECK_COND_ERROR(context, cmp_write_u8(&context->ctx, *(uint8_t *)addr));
    break;
  case cl_COLUMN_UINT16:
    UNSAFE_READ_MEMORY(addr, uint16_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u16(&context->ctx, storage.u16));
    break;
  case cl_COLUMN_UINT32:
    UNSAFE_READ_MEMORY(addr, uint32_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u32(&context->ctx, storage.u32));
    break;
  case cl_COLUMN_UINT64:
    UNSAFE_READ_MEMORY(addr, uint64_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u64(&context->ctx, storage.u64));
    break;
  default:
    assert(false);
  }
}

void unpack_addr(struct context *context, uint8_t kind, void *addr) {
  CHECK_MEMORY_BOUNDARY(context, kind, addr);

  struct full_storage storage;
  switch (kind) {
  case cl_COLUMN_BOOL:
    CHECK_COND_ERROR(context, cmp_read_bool(&context->ctx, (bool *)addr));
    break;
  case cl_COLUMN_INT8:
    CHECK_COND_ERROR(context, cmp_read_s8(&context->ctx, (int8_t *)addr));
    break;
  case cl_COLUMN_INT16:
    CHECK_COND_ERROR(context, cmp_read_s16(&context->ctx, &storage.i16));
    UNSAFE_WRITE_MEMORY(addr, int16_t, storage);
    break;
  case cl_COLUMN_INT32:
    CHECK_COND_ERROR(context, cmp_read_s32(&context->ctx, &storage.i32));
    UNSAFE_WRITE_MEMORY(addr, int32_t, storage);
    break;
  case cl_COLUMN_INT64:
    CHECK_COND_ERROR(context, cmp_read_s64(&context->ctx, &storage.i64));
    UNSAFE_WRITE_MEMORY(addr, int64_t, storage);
    break;
  case cl_COLUMN_UINT8:
    CHECK_COND_ERROR(context, cmp_read_u8(&context->ctx, (uint8_t *)addr));
    break;
  case cl_COLUMN_UINT16:
    CHECK_COND_ERROR(context, cmp_read_u16(&context->ctx, &storage.u16));
    UNSAFE_WRITE_MEMORY(addr, uint16_t, storage);
    break;
  case cl_COLUMN_UINT32:
    CHECK_COND_ERROR(context, cmp_read_u32(&context->ctx, &storage.u32));
    UNSAFE_WRITE_MEMORY(addr, uint32_t, storage);
    break;
  case cl_COLUMN_UINT64:
    CHECK_COND_ERROR(context, cmp_read_u64(&context->ctx, &storage.u64));
    UNSAFE_WRITE_MEMORY(addr, uint64_t, storage);
    break;
  default:
    assert(false);
  }
}

void read_addr(uint8_t kind, const void *addr, struct storage *storage) {
  struct full_storage full_storage;
  switch (kind) {
  case cl_COLUMN_BOOL:
    storage->i64 = *(bool *)addr;
    break;
  case cl_COLUMN_INT8:
    UNSAFE_READ_MEMORY(addr, int8_t, full_storage);
    storage->i64 = full_storage.i8;
    break;
  case cl_COLUMN_INT16:
    UNSAFE_READ_MEMORY(addr, int16_t, full_storage);
    storage->i64 = full_storage.i16;
    break;
  case cl_COLUMN_INT32:
    UNSAFE_READ_MEMORY(addr, int32_t, full_storage);
    storage->i64 = full_storage.i32;
    break;
  case cl_COLUMN_INT64:
    UNSAFE_READ_MEMORY(addr, int64_t, full_storage);
    storage->i64 = full_storage.i64;
    break;
  case cl_COLUMN_UINT8:
    UNSAFE_READ_MEMORY(addr, uint8_t, full_storage);
    storage->u64 = full_storage.u8;
    break;
  case cl_COLUMN_UINT16:
    UNSAFE_READ_MEMORY(addr, uint16_t, full_storage);
    storage->u64 = full_storage.u16;
    break;
  case cl_COLUMN_UINT32:
    UNSAFE_READ_MEMORY(addr, uint32_t, full_storage);
    storage->u64 = full_storage.u32;
    break;
  case cl_COLUMN_UINT64:
    UNSAFE_READ_MEMORY(addr, uint64_t, full_storage);
    storage->u64 = full_storage.u64;
    break;
  default:
    assert(false);
  }
}

void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    struct context *context) {
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
