#include <assert.h>
#include <string.h>

#include "common.h"

const int32_t SIZES[cl_MAX] = {
    0,            // cl_NONE
    1,            // cl_INT8
    2,            // cl_INT16
    4,            // cl_INT32
    8,            // cl_INT64
    16,           // cl_INT128
    32,           // cl_INT256
    1,            // cl_UINT8
    2,            // cl_UINT16
    4,            // cl_UINT32
    8,            // cl_UINT64
    16,           // cl_UINT128
    32,           // cl_UINT256
    1,            // cl_FLOAT8
    2,            // cl_FLOAT16
    4,            // cl_FLOAT32
    8,            // cl_FLOAT64
    16,           // cl_FLOAT128
    32,           // cl_FLOAT256
    sizeof(bool), // cl_BOOL,
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

#define CHECK_MEMORY_BOUNDARY(ctx, tp, addr)                                   \
  do {                                                                         \
    if ((uint8_t *)(addr) + SIZES[(tp)] > (ctx)->end_addr) {                   \
      (ctx)->has_error = true;                                                 \
      return;                                                                  \
    }                                                                          \
  } while (false)

void pack_addr(struct context *context, uint8_t tp, const void *addr) {
  CHECK_MEMORY_BOUNDARY(context, tp, addr);

  struct full_storage storage;
  switch (tp) {
  case cl_BOOL:
    CHECK_COND_ERROR(context, cmp_write_bool(&context->ctx, *(bool *)addr));
    break;
  case cl_INT8:
    CHECK_COND_ERROR(context, cmp_write_s8(&context->ctx, *(int8_t *)addr));
    break;
  case cl_INT16:
    UNSAFE_READ_MEMORY(addr, int16_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s16(&context->ctx, storage.i16));
    break;
  case cl_INT32:
    UNSAFE_READ_MEMORY(addr, int32_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s32(&context->ctx, storage.i32));
    break;
  case cl_INT64:
    UNSAFE_READ_MEMORY(addr, int64_t, storage);
    CHECK_COND_ERROR(context, cmp_write_s64(&context->ctx, storage.i64));
    break;
  case cl_UINT8:
    CHECK_COND_ERROR(context, cmp_write_u8(&context->ctx, *(uint8_t *)addr));
    break;
  case cl_UINT16:
    UNSAFE_READ_MEMORY(addr, uint16_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u16(&context->ctx, storage.u16));
    break;
  case cl_UINT32:
    UNSAFE_READ_MEMORY(addr, uint32_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u32(&context->ctx, storage.u32));
    break;
  case cl_UINT64:
    UNSAFE_READ_MEMORY(addr, uint64_t, storage);
    CHECK_COND_ERROR(context, cmp_write_u64(&context->ctx, storage.u64));
    break;
  default:
    assert(false);
  }
}

void unpack_addr(struct context *context, uint8_t tp, void *addr) {
  CHECK_MEMORY_BOUNDARY(context, tp, addr);

  struct full_storage storage;
  switch (tp) {
  case cl_BOOL:
    CHECK_COND_ERROR(context, cmp_read_bool(&context->ctx, (bool *)addr));
    break;
  case cl_INT8:
    CHECK_COND_ERROR(context, cmp_read_s8(&context->ctx, (int8_t *)addr));
    break;
  case cl_INT16:
    CHECK_COND_ERROR(context, cmp_read_s16(&context->ctx, &storage.i16));
    UNSAFE_WRITE_MEMORY(addr, int16_t, storage);
    break;
  case cl_INT32:
    CHECK_COND_ERROR(context, cmp_read_s32(&context->ctx, &storage.i32));
    UNSAFE_WRITE_MEMORY(addr, int32_t, storage);
    break;
  case cl_INT64:
    CHECK_COND_ERROR(context, cmp_read_s64(&context->ctx, &storage.i64));
    UNSAFE_WRITE_MEMORY(addr, int64_t, storage);
    break;
  case cl_UINT8:
    CHECK_COND_ERROR(context, cmp_read_u8(&context->ctx, (uint8_t *)addr));
    break;
  case cl_UINT16:
    CHECK_COND_ERROR(context, cmp_read_u16(&context->ctx, &storage.u16));
    UNSAFE_WRITE_MEMORY(addr, uint16_t, storage);
    break;
  case cl_UINT32:
    CHECK_COND_ERROR(context, cmp_read_u32(&context->ctx, &storage.u32));
    UNSAFE_WRITE_MEMORY(addr, uint32_t, storage);
    break;
  case cl_UINT64:
    CHECK_COND_ERROR(context, cmp_read_u64(&context->ctx, &storage.u64));
    UNSAFE_WRITE_MEMORY(addr, uint64_t, storage);
    break;
  default:
    assert(false);
  }
}

void pack_array(struct context *context, uint8_t tp, const void *addr,
                int32_t len) {
  if (tp == cl_INT8 || tp == cl_UINT8) {
    CHECK_COND_ERROR(context, cmp_write_bin(&context->ctx, addr, len));
    return;
  }

  CHECK_COND_ERROR(context, cmp_write_array(&context->ctx, len));

  ptrdiff_t offset = 0;

  for (int i = 0; i < len; ++i) {
    offset = offset + SIZES[tp] * i;

    pack_addr(context, tp, addr + offset);

    CHECK_CTX_ERROR(context);
  }
}

void unpack_array(struct context *context, uint8_t tp, void *addr,
                  int32_t len) {
  uint32_t size = len;

  if (tp == cl_INT8 || tp == cl_UINT8) {
    CHECK_COND_ERROR(context, cmp_read_bin(&context->ctx, addr, &size));
    return;
  }

  CHECK_COND_ERROR(context, cmp_read_array(&context->ctx, &size));
  CHECK_COND_ERROR(context, size == len);

  ptrdiff_t offset = 0;

  for (int i = 0; i < len; ++i) {
    offset = offset + SIZES[tp] * i;

    unpack_addr(context, tp, addr + offset);

    CHECK_CTX_ERROR(context);
  }
}

void pack_string(struct context *context, uint8_t tp, const void *addr,
                 int32_t cap) {
  switch (tp) {
  case cl_INT8:
  case cl_UINT8: {
    const int32_t len = strnlen(addr, cap - 1);
    CHECK_COND_ERROR(context, cmp_write_str(&context->ctx, addr, len));
    return;
  }
  case cl_INT16:
  case cl_UINT16:
  case cl_INT32:
  case cl_UINT32: {
    // not supported
  }
  default:
    assert(false);
  }
}

void unpack_string(struct context *context, uint8_t tp, void *addr,
                   int32_t cap) {
  switch (tp) {
  case cl_INT8:
  case cl_UINT8: {
    uint32_t size = cap;
    CHECK_COND_ERROR(context, cmp_read_str(&context->ctx, addr, &size));
    return;
  }
  case cl_INT16:
  case cl_UINT16:
  case cl_INT32:
  case cl_UINT32: {
    // not supported
  }
  default:
    assert(false);
  }
}

void read_addr(uint8_t tp, const void *addr, struct storage *storage) {
  struct full_storage full_storage;
  switch (tp) {
  case cl_BOOL:
    storage->i64 = *(bool *)addr;
    break;
  case cl_INT8:
    UNSAFE_READ_MEMORY(addr, int8_t, full_storage);
    storage->i64 = full_storage.i8;
    break;
  case cl_INT16:
    UNSAFE_READ_MEMORY(addr, int16_t, full_storage);
    storage->i64 = full_storage.i16;
    break;
  case cl_INT32:
    UNSAFE_READ_MEMORY(addr, int32_t, full_storage);
    storage->i64 = full_storage.i32;
    break;
  case cl_INT64:
    UNSAFE_READ_MEMORY(addr, int64_t, full_storage);
    storage->i64 = full_storage.i64;
    break;
  case cl_UINT8:
    UNSAFE_READ_MEMORY(addr, uint8_t, full_storage);
    storage->u64 = full_storage.u8;
    break;
  case cl_UINT16:
    UNSAFE_READ_MEMORY(addr, uint16_t, full_storage);
    storage->u64 = full_storage.u16;
    break;
  case cl_UINT32:
    UNSAFE_READ_MEMORY(addr, uint32_t, full_storage);
    storage->u64 = full_storage.u32;
    break;
  case cl_UINT64:
    UNSAFE_READ_MEMORY(addr, uint64_t, full_storage);
    storage->u64 = full_storage.u64;
    break;
  default:
    assert(false);
  }
}

void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    struct context *context) {
  switch (column->tp) {
  case cl_INT8:
  case cl_INT16:
  case cl_INT32:
  case cl_INT64:
  case cl_INT128:
  case cl_INT256:
  case cl_UINT8:
  case cl_UINT16:
  case cl_UINT32:
  case cl_UINT64:
  case cl_UINT128:
  case cl_UINT256:
  case cl_FLOAT8:
  case cl_FLOAT16:
  case cl_FLOAT32:
  case cl_FLOAT64:
  case cl_FLOAT128:
  case cl_FLOAT256:
  case cl_BOOL:
    return visitor->visit_number(visitor, column, context);

  case cl_OBJECT:
    return visitor->visit_object(visitor, column, context);

  case cl_UNION:
    return visitor->visit_union(visitor, column, context);

  case cl_FIXED_ARRAY:
    return visitor->visit_fixed_array(visitor, column, context);

  case cl_FLEXIBLE_ARRAY:
    return visitor->visit_flexible_array(visitor, column, context);

  case cl_STRING:
    return visitor->visit_string(visitor, column, context);

  default:
    assert(false);
  }
}
