#include <string.h>

#include "internal.h"

struct encoder {
  struct context base;
  const uint8_t *base_addr;

  size_t capacity;
  size_t wpos;
  uint8_t *buf;
};

static inline size_t write_buf(cmp_ctx_t *context, const void *data,
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
  const clColumn *columns = column->via_object.fields;

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

  CHECK_COND_ERROR(&encoder->base, pos <= column->via_object.num);
  CHECK_COND_ERROR(&encoder->base, cmp_write_u32(&encoder->base.ctx, pos));

  if (!pos) {
    return;
  }

  visit_children(visitor, column->via_union.fields + pos - 1, &encoder->base);
}

static inline void visit_fixed_array(const struct visitor_ops *visitor,
                                     const clColumn *column,
                                     struct encoder *encoder) {
  visit_array(visitor, column->via_fixed_array.capacity,
              column->via_fixed_array.flags, column->via_fixed_array.element,
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
              column->via_flexible_array.element, encoder);
}

static struct visitor_ops visitor = {
    .visit_number = (visitor_handler)visit_number,
    .visit_object = (visitor_handler)visit_object,
    .visit_union = (visitor_handler)visit_union,
    .visit_fixed_array = (visitor_handler)visit_fixed_array,
    .visit_flexible_array = (visitor_handler)visit_flexible_array,
};

size_t cToBuf(const clColumn *column, const void *addr, size_t size,
              uint8_t *buf, size_t len) {
  struct encoder encoder = {
      .base =
          {
              .end_addr = (const uint8_t *)addr + size,
          },
      .base_addr = (const uint8_t *)addr,
      .capacity = len,
      .buf = buf,
  };

  cmp_init(&encoder.base.ctx, &encoder, NULL, NULL, write_buf);

  visit_children(&visitor, column, &encoder.base);
  if (encoder.base.has_error) {
    return 0;
  }

  return encoder.wpos;
}
