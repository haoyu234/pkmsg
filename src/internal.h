#pragma once

#include <columns.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define CHECK_COND_ERROR(ctx, cond)                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      ctx->has_error = true;                                                   \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define CHECK_CTX_ERROR(ctx)                                                   \
  do {                                                                         \
    if (ctx->has_error) {                                                      \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define SIZE(kind) (1ul << (((kind % 6) ? (kind % 6) : 6) - 1))

#define CHECK_MEMORY(ctx, kind)                                                \
  do {                                                                         \
    if (ctx->offset + SIZE(kind) > ctx->size) {                                \
      ctx->has_error = true;                                                   \
      return;                                                                  \
    }                                                                          \
  } while (false)

struct storage_union {
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

#define UNSAFE_READ_MEMORY(ptr, type, storage)                                 \
  memcpy(&storage, ptr, sizeof(type))

#define UNSAFE_WRITE_MEMORY(ptr, type, value)                                  \
  do {                                                                         \
    const type storage = value;                                                \
    memcpy(ptr, &storage, sizeof(type));                                       \
  } while (false)

struct visitor_ops;

typedef void (*visitor_handler)(const struct visitor_ops *visitor,
                                const clColumn *column, void *context);

struct visitor_ops {
  visitor_handler visit_number;
  visitor_handler visit_object;
  visitor_handler visit_union;
  visitor_handler visit_fixed_array;
  visitor_handler visit_flexible_array;
};

uint32_t read_u32(uint8_t kind, const void *addr);
void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    void *context);
