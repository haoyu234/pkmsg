#include <string.h>

#include "common.h"

struct encoder {
  struct context base;
  const uint8_t *base_addr;

  int32_t wpos;
  union {
    struct {
      int32_t capacity;
      uint8_t *buf;
    };
    struct {
      void *ctx;
      int32_t (*write_cb)(void *ctx, const void *data, int32_t count);
    };
  };
};

static inline size_t write_buf1(cmp_ctx_t *context, const void *data,
                                size_t count) {
  struct encoder *encoder = (struct encoder *)context->buf;
  const int32_t r = encoder->write_cb(encoder->ctx, data, count);
  encoder->wpos += r;
  return r;
}

static inline size_t write_buf2(cmp_ctx_t *context, const void *data,
                                size_t count) {
  struct encoder *encoder = (struct encoder *)context->buf;

  if (encoder->wpos + count > encoder->capacity) {
    return 0;
  }

  memcpy(encoder->buf + encoder->wpos, data, count);

  encoder->wpos += count;
  return count;
}

static inline void pack_number(const struct visitor_ops *visitor, uint8_t kind,
                               struct encoder *encoder) {
  const ptrdiff_t offset = encoder->base.offset;
  pack_addr(&encoder->base, kind, encoder->base_addr + offset);
}

static inline void visit_array(const struct visitor_ops *visitor, uint32_t num,
                               uint8_t kind, const clColumn *element,
                               struct encoder *encoder) {
  CHECK_COND_ERROR(&encoder->base, cmp_write_array(&encoder->base.ctx, num));

  if (!num) {
    return;
  }

  const ptrdiff_t offset = encoder->base.offset;

  if (element != NULL) {
    const uint32_t stride = element->size;

    for (int i = 0; i < num; ++i) {
      encoder->base.offset = offset + stride * i;

      visit_children(visitor, element, &encoder->base);

      CHECK_CTX_ERROR(&encoder->base);
    }

    return;
  }

  for (int i = 0; i < num; ++i) {
    encoder->base.offset = offset + SIZE(kind) * i;

    pack_number(visitor, kind, encoder);

    CHECK_CTX_ERROR(&encoder->base);
  }
}

static void visit_number(const struct visitor_ops *visitor,
                         const clColumn *column, struct encoder *encoder) {
  pack_number(visitor, column->kind, encoder);
}

static inline void visit_object(const struct visitor_ops *visitor,
                                const clColumn *column,
                                struct encoder *encoder) {
  const ptrdiff_t offset = encoder->base.offset;

  const uint32_t num = column->via_object.num;
  const clColumn *columns = column->via_object.columns;

  CHECK_COND_ERROR(&encoder->base, cmp_write_array(&encoder->base.ctx, num));

  for (int i = 0; i < num; ++i) {
    encoder->base.offset = offset + columns[i].offset;

    visit_children(visitor, columns + i, &encoder->base);

    CHECK_CTX_ERROR(&encoder->base);
  }
}

static inline void visit_union(const struct visitor_ops *visitor,
                               const clColumn *column,
                               struct encoder *encoder) {
  const ptrdiff_t offset = encoder->base.offset;

  const clColumn *prev_column = column - 1;

  const void *addr =
      encoder->base_addr + offset + prev_column->offset - column->offset;

  struct storage storage;
  read_addr(prev_column->kind, addr, &storage);

  const uint32_t pos = storage.u64;

  CHECK_COND_ERROR(&encoder->base, pos <= column->via_union.num);
  CHECK_COND_ERROR(&encoder->base, cmp_write_u32(&encoder->base.ctx, pos));

  if (!pos) {
    return;
  }

  visit_children(visitor, column->via_union.columns + pos - 1, &encoder->base);
}

static inline void visit_fixed_array(const struct visitor_ops *visitor,
                                     const clColumn *column,
                                     struct encoder *encoder) {
  visit_array(visitor, column->via_fixed_array.capacity,
              column->via_fixed_array.flags, column->via_fixed_array.columns,
              encoder);
}

static inline void visit_flexible_array(const struct visitor_ops *visitor,
                                        const clColumn *column,
                                        struct encoder *encoder) {
  const ptrdiff_t offset = encoder->base.offset;

  const clColumn *prev_column = column - 1;

  const void *addr =
      encoder->base_addr + offset + prev_column->offset - column->offset;

  struct storage storage;
  read_addr(prev_column->kind, addr, &storage);

  const uint32_t num = storage.u64;

  CHECK_COND_ERROR(&encoder->base, num <= column->via_flexible_array.capacity);

  visit_array(visitor, num, column->via_flexible_array.flags,
              column->via_flexible_array.columns, encoder);
}

static struct visitor_ops visitor = {
    .visit_number = (visitor_handler)visit_number,
    .visit_object = (visitor_handler)visit_object,
    .visit_union = (visitor_handler)visit_union,
    .visit_fixed_array = (visitor_handler)visit_fixed_array,
    .visit_flexible_array = (visitor_handler)visit_flexible_array,
};

int32_t pk_encode_cb(const struct clColumn *column, const void *addr,
                     int32_t size, void *ctx,
                     int32_t (*write_cb)(void *ctx, const void *data,
                                         int32_t count)) {
  struct encoder encoder = {
      .base =
          {
              .end_addr = (const uint8_t *)addr + size,
          },
      .base_addr = (const uint8_t *)addr,
      .ctx = ctx,
      .write_cb = write_cb,
  };

  cmp_init(&encoder.base.ctx, &encoder, NULL, NULL, write_buf1);

  visit_children(&visitor, column, &encoder.base);
  if (encoder.base.has_error) {
    return 0;
  }

  return encoder.wpos;
}

// struct buf_s {
//   int32_t wpos;
//   int32_t capacity;
//   uint8_t *buf;
// };

// static inline int32_t write_buf3(void *context, const void *data,
//                                  int32_t count) {
//   struct buf_s *buf = (struct buf_s *)context;

//   if (buf->wpos + count > buf->capacity) {
//     return false;
//   }

//   memcpy(buf->buf + buf->wpos, data, count);

//   buf->wpos += count;
//   return count;
// }

// int32_t pk_encode(const clColumn *column, const void *addr, int32_t size,
//                   uint8_t *buf, int32_t len) {
//   struct buf_s ctx = {
//       .buf = buf,
//       .capacity = len,
//   };

//   return pk_encode_cb(column, addr, size, &ctx, write_buf3);
// }

int32_t pk_encode(const clColumn *column, const void *addr, int32_t size,
                  uint8_t *buf, int32_t len) {
  struct encoder encoder = {
      .base =
          {
              .end_addr = (const uint8_t *)addr + size,
          },
      .base_addr = (const uint8_t *)addr,
      .capacity = len,
      .buf = buf,
  };

  cmp_init(&encoder.base.ctx, &encoder, NULL, NULL, write_buf2);

  visit_children(&visitor, column, &encoder.base);
  if (encoder.base.has_error) {
    return 0;
  }

  return encoder.wpos;
}
