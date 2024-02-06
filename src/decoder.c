#include <string.h>

#include "common.h"

struct decoder {
  struct context base;
  uint8_t *base_addr;

  size_t capacity;
  size_t rpos;
  const uint8_t *buf;
};

static inline bool read_buf(cmp_ctx_t *context, void *data, size_t count) {
  struct decoder *decoder = (struct decoder *)context->buf;

  if (decoder->rpos + count > decoder->capacity) {
    return false;
  }

  memcpy(data, decoder->buf + decoder->rpos, count);

  decoder->rpos += count;
  return true;
}

static inline void unpack_number(const struct visitor_ops *visitor,
                                 uint8_t kind, struct decoder *decoder) {
  const ptrdiff_t offset = decoder->base.offset;
  unpack_addr(&decoder->base, kind, decoder->base_addr + offset);
}

static inline void visit_array(const struct visitor_ops *visitor, uint32_t num,
                               uint8_t kind, const clColumn *element,
                               struct decoder *decoder) {
  uint32_t real_num = 0;
  CHECK_COND_ERROR(&decoder->base,
                   cmp_read_array(&decoder->base.ctx, &real_num));
  CHECK_COND_ERROR(&decoder->base, num == real_num);

  if (!num) {
    return;
  }

  const ptrdiff_t offset = decoder->base.offset;

  if (element != NULL) {
    const uint32_t stride = element->size;

    for (int i = 0; i < num; ++i) {
      decoder->base.offset = offset + stride * i;

      visit_children(visitor, element, &decoder->base);

      CHECK_CTX_ERROR(&decoder->base);
    }

    return;
  }

  for (int i = 0; i < num; ++i) {
    decoder->base.offset = offset + SIZE(kind) * i;

    unpack_number(visitor, kind, decoder);

    CHECK_CTX_ERROR(&decoder->base);
  }
}

static void visit_number(const struct visitor_ops *visitor,
                         const clColumn *column, struct decoder *decoder) {
  unpack_number(visitor, column->kind, decoder);
}

static inline void visit_object(const struct visitor_ops *visitor,
                                const clColumn *column,
                                struct decoder *decoder) {
  const uint32_t num = column->via_object.num;
  const ptrdiff_t offset = decoder->base.offset;

  uint32_t size;
  CHECK_COND_ERROR(&decoder->base, cmp_read_array(&decoder->base.ctx, &size));
  CHECK_COND_ERROR(&decoder->base, size == column->via_object.num);

  const clColumn *columns = column->via_object.columns;

  for (int i = 0; i < num; ++i) {
    decoder->base.offset = offset + columns[i].offset;

    visit_children(visitor, columns + i, &decoder->base);

    CHECK_CTX_ERROR(&decoder->base);
  }
}

static inline void visit_union(const struct visitor_ops *visitor,
                               const clColumn *column,
                               struct decoder *decoder) {
  uint32_t num = 0;
  CHECK_COND_ERROR(&decoder->base, cmp_read_u32(&decoder->base.ctx, &num));
  CHECK_COND_ERROR(&decoder->base, num <= column->via_union.num);

  const ptrdiff_t offset = decoder->base.offset;

  const clColumn *prev_column = column - 1;

  const void *addr =
      decoder->base_addr + offset + prev_column->offset - column->offset;

  struct storage storage;
  read_addr(prev_column->kind, addr, &storage);

  const uint32_t pos = storage.u64;

  CHECK_COND_ERROR(&decoder->base, pos <= column->via_union.num);

  if (!pos) {
    return;
  }

  visit_children(visitor, column->via_union.columns + pos - 1, &decoder->base);
}

static inline void visit_fixed_array(const struct visitor_ops *visitor,
                                     const clColumn *column,
                                     struct decoder *decoder) {
  visit_array(visitor, column->via_fixed_array.capacity,
              column->via_fixed_array.flags, column->via_fixed_array.columns,
              decoder);
}

static inline void visit_flexible_array(const struct visitor_ops *visitor,
                                        const clColumn *column,
                                        struct decoder *decoder) {

  const ptrdiff_t offset = decoder->base.offset;

  const clColumn *prev_column = column - 1;

  const void *addr =
      decoder->base_addr + offset + prev_column->offset - column->offset;

  struct storage storage;
  read_addr(prev_column->kind, addr, &storage);

  const uint32_t num = storage.u64;

  CHECK_COND_ERROR(&decoder->base, num <= column->via_flexible_array.capacity);

  visit_array(visitor, num, column->via_flexible_array.flags,
              column->via_flexible_array.columns, decoder);
}

static struct visitor_ops visitor = {
    .visit_number = (visitor_handler)visit_number,
    .visit_object = (visitor_handler)visit_object,
    .visit_union = (visitor_handler)visit_union,
    .visit_fixed_array = (visitor_handler)visit_fixed_array,
    .visit_flexible_array = (visitor_handler)visit_flexible_array,
};

size_t cFromBuf(const clColumn *column, void *addr, size_t size,
                const uint8_t *buf, size_t len) {
  struct decoder decoder = {
      .base =
          {
              .end_addr = (const uint8_t *)addr + size,
          },
      .base_addr = (uint8_t *)addr,
      .capacity = len,
      .buf = buf,
  };

  cmp_init(&decoder.base.ctx, &decoder, read_buf, NULL, NULL);

  visit_children(&visitor, column, &decoder.base);
  if (decoder.base.has_error) {
    return 0;
  }

  return decoder.rpos;
}
