#pragma once

#include <cmp.h>
#include <columns.h>
#include <pkmsg.h>

#ifdef _DEBUG
#include <assert.h>
#define ASSERT_FALSE() assert(false)
#else
#define ASSERT_FALSE()
#endif

#define CHECK_COND_ERROR(ctx, cond)                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      ASSERT_FALSE();                                                          \
      (ctx)->has_error = true;                                                 \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define CHECK_CTX_ERROR(ctx)                                                   \
  do {                                                                         \
    if ((ctx)->has_error) {                                                    \
      return;                                                                  \
    }                                                                          \
  } while (false)

struct storage {
  union {
    uint64_t u64;
    int64_t i64;
  };
};

struct context {
  bool has_error;

  ptrdiff_t offset;
  const uint8_t *end_addr;

  cmp_ctx_t ctx;
};

struct visitor_ops;

typedef void (*visitor_handler)(const struct visitor_ops *visitor,
                                const clColumn *column,
                                struct context *context);

struct visitor_ops {
  visitor_handler visit_number;
  visitor_handler visit_object;
  visitor_handler visit_union;
  visitor_handler visit_fixed_array;
  visitor_handler visit_flexible_array;
  visitor_handler visit_string;
};

void pack_addr(struct context *context, uint8_t tp, const void *addr);
void unpack_addr(struct context *context, uint8_t tp, void *addr);

void pack_array(struct context *context, uint8_t tp, const void *addr,
                int32_t len);
void unpack_array(struct context *context, uint8_t tp, void *addr, int32_t len);

void pack_string(struct context *context, uint8_t tp, const void *addr,
                 int32_t cap);
void unpack_string(struct context *context, uint8_t tp, void *addr,
                   int32_t cap);

void read_addr(uint8_t tp, const void *addr, struct storage *storage);

void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    struct context *context);
