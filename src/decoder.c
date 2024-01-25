#include <columns.h>
#include <msgpack.h>

#include "internal.h"

struct decoder {
  bool has_error;

  uint8_t *addr;
  size_t size;
  ptrdiff_t offset;

  msgpack_object *object;
};

static inline void visit_number(const struct visitor_ops *visitor, uint8_t kind,
                                struct decoder *decoder) {
  const msgpack_object *object = decoder->object;
  const ptrdiff_t offset = decoder->offset;

  CHECK_MEMORY(decoder, kind);

  CHECK_COND_ERROR(decoder,
                   object->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
                       object->type == MSGPACK_OBJECT_NEGATIVE_INTEGER ||
                       object->type == MSGPACK_OBJECT_BOOLEAN ||
                       object->type == MSGPACK_OBJECT_FLOAT32 ||
                       object->type == MSGPACK_OBJECT_FLOAT64 ||
                       object->type == MSGPACK_OBJECT_FLOAT);

  switch (kind) {
  case cl_COLUMN_INT8:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, int8_t, object->via.i64);
    break;
  case cl_COLUMN_INT16:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, int16_t, object->via.i64);
    break;
  case cl_COLUMN_INT32:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, int32_t, object->via.i64);
    break;
  case cl_COLUMN_INT64:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, int64_t, object->via.i64);
    break;
  case cl_COLUMN_UINT8:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint8_t, object->via.u64);
    break;
  case cl_COLUMN_UINT16:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint16_t, object->via.u64);
    break;
  case cl_COLUMN_UINT32:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint32_t, object->via.u64);
    break;
  case cl_COLUMN_UINT64:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint64_t, object->via.u64);
    break;
  case cl_COLUMN_FLOAT32:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, float, object->via.f64);
    break;
  case cl_COLUMN_FLOAT64:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, double, object->via.f64);
    break;
  case cl_COLUMN_BOOL:
    UNSAFE_WRITE_MEMORY(decoder->addr + offset, bool, object->via.boolean);
    break;
  default:
    assert(false);
    break;
  }
}

static inline void visit_array(const struct visitor_ops *visitor, uint32_t num,
                               uint8_t kind, const clColumn *element,
                               struct decoder *decoder) {
  const msgpack_object *object = decoder->object;
  const ptrdiff_t offset = decoder->offset;

  uint32_t i = 0;

  if (element != NULL) {
    const uint32_t stride = element->size;

    for (i = 0; i < num; ++i) {
      decoder->object = object->via.array.ptr + i;
      decoder->offset = offset + stride * i;

      visit_children(visitor, element, decoder);

      CHECK_CTX_ERROR(decoder);
    }
  } else {
    for (i = 0; i < num; ++i) {
      decoder->object = object->via.array.ptr + i;
      decoder->offset = offset + SIZE(kind) * i;

      visit_number(visitor, kind, decoder);

      CHECK_CTX_ERROR(decoder);
    }
  }
}

static void visit_number_handler(const struct visitor_ops *visitor,
                                 const clColumn *column,
                                 struct decoder *decoder) {
  visit_number(visitor, column->kind, decoder);
}

static inline void visit_object_handler(const struct visitor_ops *visitor,
                                        const clColumn *column,
                                        struct decoder *decoder) {
  const msgpack_object *object = decoder->object;
  const ptrdiff_t offset = decoder->offset;

  CHECK_COND_ERROR(decoder,
                   object->type == MSGPACK_OBJECT_ARRAY &&
                       object->via.array.size == column->via_object.num);

  const uint32_t num = column->via_object.num;
  const clColumn *columns = column->via_object.fields;

  uint32_t i = 0;

  for (i = 0; i < num; ++i) {
    decoder->object = object->via.array.ptr + i;
    decoder->offset = offset + columns[i].offset;

    visit_children(visitor, columns + i, decoder);

    CHECK_CTX_ERROR(decoder);
  }
}

static inline void visit_union_handler(const struct visitor_ops *visitor,
                                       const clColumn *column,
                                       struct decoder *decoder) {
  const ptrdiff_t offset = decoder->offset;

  const clColumn *prev_column = column - 1;

  const uint32_t pos =
      read_u32(prev_column->kind,
               decoder->addr + offset + prev_column->offset - column->offset);

  CHECK_COND_ERROR(decoder, pos && pos <= column->via_object.num);

  visit_children(visitor, column->via_union.fields + pos - 1, decoder);
}

static inline void visit_fixed_array_handler(const struct visitor_ops *visitor,
                                             const clColumn *column,
                                             struct decoder *decoder) {
  const msgpack_object *object = decoder->object;

  CHECK_COND_ERROR(decoder, object->type == MSGPACK_OBJECT_ARRAY &&
                                object->via.array.size ==
                                    column->via_fixed_array.capacity);

  visit_array(visitor, column->via_fixed_array.capacity,
              column->via_fixed_array.flags, column->via_fixed_array.element,
              decoder);
}

static inline void
visit_flexible_array_handler(const struct visitor_ops *visitor,
                             const clColumn *column, struct decoder *decoder) {
  const msgpack_object *object = decoder->object;
  const ptrdiff_t offset = decoder->offset;

  CHECK_COND_ERROR(decoder, object->type == MSGPACK_OBJECT_ARRAY);

  const clColumn *prev_column = column - 1;

  const uint32_t num =
      read_u32(prev_column->kind,
               decoder->addr + offset + prev_column->offset - column->offset);

  CHECK_COND_ERROR(decoder, object->via.array.size == num);

  visit_array(visitor, object->via.array.size, column->via_flexible_array.flags,
              column->via_flexible_array.element, decoder);
}

static struct visitor_ops visitor = {
    .visit_number = (visitor_handler)visit_number_handler,
    .visit_object = (visitor_handler)visit_object_handler,
    .visit_union = (visitor_handler)visit_union_handler,
    .visit_fixed_array = (visitor_handler)visit_fixed_array_handler,
    .visit_flexible_array = (visitor_handler)visit_flexible_array_handler,
};

size_t cFromBuf(const clColumn *column, void *addr, size_t size,
                const uint8_t *buf, size_t len) {
  msgpack_unpacked unpacked;
  msgpack_unpacked_init(&unpacked);

  size_t offset = 0;
  msgpack_unpack_return ret =
      msgpack_unpack_next(&unpacked, (const char *)buf, len, &offset);

  struct decoder decoder = {.has_error = (ret != MSGPACK_UNPACK_SUCCESS),
                            .addr = (uint8_t *)addr,
                            .size = size,
                            .offset = 0,
                            .object = &unpacked.data};

  if (!decoder.has_error) {
    visit_children(&visitor, column, &decoder);
  }

  msgpack_unpacked_destroy(&unpacked);

  return !decoder.has_error ? offset : 0;
}
