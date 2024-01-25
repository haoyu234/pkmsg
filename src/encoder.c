#include <columns.h>
#include <msgpack.h>

#include "internal.h"

struct encoder {
  bool has_error;

  const uint8_t *addr;
  size_t size;
  ptrdiff_t offset;

  size_t capacity;
  size_t wpos;
  uint8_t *buf;

  msgpack_packer packer;
};

static inline int write_buf(void *context, const char *buf, size_t len) {
  struct encoder *encoder = (struct encoder *)context;

  if (encoder->wpos + len > encoder->capacity) {
    return -1;
  }

  memcpy(encoder->buf + encoder->wpos, buf, len);

  encoder->wpos += len;
  return 0;
}

static inline void visit_number(const struct visitor_ops *visitor, uint8_t kind,
                                struct encoder *encoder) {
  struct storage_union storage;
  const ptrdiff_t offset = encoder->offset;

  CHECK_MEMORY(encoder, kind);

  switch (kind) {
  case cl_COLUMN_INT8:
    UNSAFE_READ_MEMORY(encoder->addr + offset, int8_t, storage);
    msgpack_pack_int8(&encoder->packer, storage.i8);
    break;
  case cl_COLUMN_INT16:
    UNSAFE_READ_MEMORY(encoder->addr + offset, int16_t, storage);
    msgpack_pack_int16(&encoder->packer, storage.i16);
    break;
  case cl_COLUMN_INT32:
    UNSAFE_READ_MEMORY(encoder->addr + offset, int32_t, storage);
    msgpack_pack_int32(&encoder->packer, storage.i32);
    break;
  case cl_COLUMN_INT64:
    UNSAFE_READ_MEMORY(encoder->addr + offset, int64_t, storage);
    msgpack_pack_int64(&encoder->packer, storage.i64);
    break;
  case cl_COLUMN_UINT8:
    UNSAFE_READ_MEMORY(encoder->addr + offset, uint8_t, storage);
    msgpack_pack_uint8(&encoder->packer, storage.u8);
    break;
  case cl_COLUMN_UINT16:
    UNSAFE_READ_MEMORY(encoder->addr + offset, uint16_t, storage);
    msgpack_pack_uint16(&encoder->packer, storage.u16);
    break;
  case cl_COLUMN_UINT32:
    UNSAFE_READ_MEMORY(encoder->addr + offset, uint32_t, storage);
    msgpack_pack_uint32(&encoder->packer, storage.u32);
    break;
  case cl_COLUMN_UINT64:
    UNSAFE_READ_MEMORY(encoder->addr + offset, uint64_t, storage);
    msgpack_pack_uint64(&encoder->packer, storage.u64);
    break;
  case cl_COLUMN_FLOAT32:
    UNSAFE_READ_MEMORY(encoder->addr + offset, float, storage);
    msgpack_pack_float(&encoder->packer, storage.f32);
    break;
  case cl_COLUMN_FLOAT64:
    UNSAFE_READ_MEMORY(encoder->addr + offset, double, storage);
    msgpack_pack_double(&encoder->packer, storage.f64);
    break;
  case cl_COLUMN_BOOL:
    UNSAFE_READ_MEMORY(encoder->addr + offset, bool, storage);
    storage.b ? msgpack_pack_true(&encoder->packer)
              : msgpack_pack_false(&encoder->packer);
    break;
  default:
    assert(false);
    break;
  }
}

static inline void visit_array(const struct visitor_ops *visitor, uint32_t num,
                               uint8_t kind, const clColumn *element,
                               struct encoder *encoder) {
  const ptrdiff_t offset = encoder->offset;

  CHECK_COND_ERROR(encoder, msgpack_pack_array(&encoder->packer, num) >= 0);

  uint32_t i = 0;

  if (element != NULL) {
    const uint32_t stride = element->size;

    for (i = 0; i < num; ++i) {
      encoder->offset = offset + stride * i;

      visit_children(visitor, element, encoder);

      CHECK_CTX_ERROR(encoder);
    }
  } else {
    for (i = 0; i < num; ++i) {
      encoder->offset = offset + SIZE(kind) * i;

      visit_number(visitor, kind, encoder);

      CHECK_CTX_ERROR(encoder);
    }
  }
}

static void visit_number_handler(const struct visitor_ops *visitor,
                                 const clColumn *column,
                                 struct encoder *encoder) {
  visit_number(visitor, column->kind, encoder);
}

static inline void visit_object_handler(const struct visitor_ops *visitor,
                                        const clColumn *column,
                                        struct encoder *encoder) {
  const ptrdiff_t offset = encoder->offset;

  const uint32_t num = column->via_object.num;
  const clColumn *columns = column->via_object.fields;

  CHECK_COND_ERROR(encoder, msgpack_pack_array(&encoder->packer, num) >= 0);

  uint32_t i = 0;

  for (i = 0; i < num; ++i) {
    encoder->offset = offset + columns[i].offset;

    visit_children(visitor, columns + i, encoder);

    CHECK_CTX_ERROR(encoder);
  }
}

static inline void visit_union_handler(const struct visitor_ops *visitor,
                                       const clColumn *column,
                                       struct encoder *encoder) {
  const ptrdiff_t offset = encoder->offset;

  const clColumn *prev_column = column - 1;

  const uint32_t pos =
      read_u32(prev_column->kind,
               encoder->addr + offset + prev_column->offset - column->offset);

  CHECK_COND_ERROR(encoder, pos && pos <= column->via_object.num);

  visit_children(visitor, column->via_union.fields + pos - 1, encoder);
}

static inline void visit_fixed_array_handler(const struct visitor_ops *visitor,
                                             const clColumn *column,
                                             struct encoder *encoder) {
  visit_array(visitor, column->via_fixed_array.capacity,
              column->via_fixed_array.flags, column->via_fixed_array.element,
              encoder);
}

static inline void
visit_flexible_array_handler(const struct visitor_ops *visitor,
                             const clColumn *column, struct encoder *encoder) {
  const ptrdiff_t offset = encoder->offset;

  const clColumn *prev_column = column - 1;

  const uint32_t num =
      read_u32(prev_column->kind,
               encoder->addr + offset + prev_column->offset - column->offset);

  visit_array(visitor, num, column->via_flexible_array.flags,
              column->via_flexible_array.element, encoder);
}

static struct visitor_ops visitor = {
    .visit_number = (visitor_handler)visit_number_handler,
    .visit_object = (visitor_handler)visit_object_handler,
    .visit_union = (visitor_handler)visit_union_handler,
    .visit_fixed_array = (visitor_handler)visit_fixed_array_handler,
    .visit_flexible_array = (visitor_handler)visit_flexible_array_handler,
};

size_t cToBuf(const clColumn *column, const void *addr, size_t size,
              uint8_t *buf, size_t len) {
  struct encoder encoder = {.has_error = false,
                            .addr = (const uint8_t *)addr,
                            .size = size,
                            .offset = 0,
                            .capacity = len,
                            .wpos = 0,
                            .buf = buf};

  msgpack_packer_init(&encoder.packer, &encoder, write_buf);

  visit_children(&visitor, column, &encoder);

  return !encoder.has_error ? encoder.wpos : 0;
}
