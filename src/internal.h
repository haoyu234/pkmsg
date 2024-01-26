#pragma once

#include <assert.h>
#include <cmp.h>
#include <columns.h>

#define CHECK_COND_ERROR(ctx, cond)                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      (ctx)->has_error = true;                                                 \
      assert(false);                                                           \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define CHECK_CTX_ERROR(ctx)                                                   \
  do {                                                                         \
    if ((ctx)->has_error) {                                                    \
      return;                                                                  \
    }                                                                          \
  } while (false)

extern const int SIZES[];
#define SIZE(kind) SIZES[kind]

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
};

void pack_addr(struct context *context, uint8_t kind, const void *addr);
void unpack_addr(struct context *context, uint8_t kind, void *addr);

void read_addr(uint8_t kind, const void *addr, struct storage *storage);

void visit_children(const struct visitor_ops *visitor, const clColumn *column,
                    struct context *context);
