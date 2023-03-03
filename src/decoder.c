#include <columns.h>
#include <msgpack.h>

#include "internal.h"

struct Decoder
{
    bool hasError;

    uint8_t *addr;
    size_t size;
    ptrdiff_t offset;

    msgpack_object *object;
};

static inline void VisitNumber(
    const clHandler *handler, 
    uint8_t kind,
    struct Decoder *decoder)
{
    const msgpack_object *object = decoder->object;
    const ptrdiff_t offset = decoder->offset;

    CHECK_MEMORY(
        decoder,
        kind);

    CHECK_COND_ERROR(
        decoder,
        object->type == MSGPACK_OBJECT_POSITIVE_INTEGER 
        || object->type == MSGPACK_OBJECT_NEGATIVE_INTEGER
        || object->type == MSGPACK_OBJECT_BOOLEAN
        || object->type == MSGPACK_OBJECT_FLOAT32
        || object->type == MSGPACK_OBJECT_FLOAT64
        || object->type == MSGPACK_OBJECT_FLOAT);

    switch (kind)
    {
        case cl_COLUMN_INT8:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, int8_t, object->via.i64);
            break ;
        case cl_COLUMN_INT16:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, int16_t, object->via.i64);
            break ;
        case cl_COLUMN_INT32:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, int32_t, object->via.i64);
            break ;
        case cl_COLUMN_INT64:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, int64_t, object->via.i64);
            break ;
        case cl_COLUMN_UINT8:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint8_t, object->via.u64);
            break ;
        case cl_COLUMN_UINT16:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint16_t, object->via.u64);
            break ;
        case cl_COLUMN_UINT32:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint32_t, object->via.u64);
            break ;
        case cl_COLUMN_UINT64:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, uint64_t, object->via.u64);
            break ;
        case cl_COLUMN_FLOAT32:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, float, object->via.f64);
            break;
        case cl_COLUMN_FLOAT64:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, double, object->via.f64);
            break;
        case cl_COLUMN_BOOL:
            UNSAFE_WRITE_MEMORY(decoder->addr + offset, bool, object->via.boolean);
            break ;
        default:
            assert(false);
            break ;
    }
}

static inline void VisitArray(
    const clHandler *handler, 
    uint32_t num,
    uint8_t kind,
    const clColumn *element,
    struct Decoder *decoder)
{
    const msgpack_object *object = decoder->object;
    const ptrdiff_t offset = decoder->offset;
    
    uint32_t i = 0;

    if (element != NULL)
    {
        const uint32_t stride = element->size;

        for (i = 0; i < num; ++ i)
        {
            decoder->object = object->via.array.ptr + i;
            decoder->offset = offset + stride * i;

            clVisitChildren(
                handler, 
                element, 
                decoder);

            CHECK_CTX_ERROR(decoder);
        }
    }
    else
    {
        for (i = 0; i < num; ++ i)
        {
            decoder->object = object->via.array.ptr + i;
            decoder->offset = offset + SIZE(kind) * i;

            VisitNumber(
                handler, 
                kind,
                decoder);

            CHECK_CTX_ERROR(decoder);
        }
    }
}

static void VisitNumberHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Decoder *decoder)
{
    VisitNumber(
        handler,
        column->kind,
        decoder);
}

static inline void VisitObjectHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Decoder *decoder)
{
    const msgpack_object *object = decoder->object;
    const ptrdiff_t offset = decoder->offset;

    CHECK_COND_ERROR(
        decoder,
        object->type == MSGPACK_OBJECT_ARRAY
        && object->via.array.size == column->viaObject.num);

    const uint32_t num = column->viaObject.num;
    const clColumn *columns = column->viaObject.fields;

    uint32_t i = 0;

    for (i = 0; i < num; ++ i)
    {
        decoder->object = object->via.array.ptr + i;
        decoder->offset = offset + columns[i].offset;

        clVisitChildren(
            handler, 
            columns + i, 
            decoder);

        CHECK_CTX_ERROR(decoder);
    }
}

static inline void VisitUnionHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Decoder *decoder)
{
    const ptrdiff_t offset = decoder->offset;

    const clColumn *prev_column = column - 1;

    const uint32_t pos = ReadMemoryAsUInt32(
        prev_column->kind, 
        decoder->addr + offset + prev_column->offset - column->offset);

    CHECK_COND_ERROR(
        decoder,
        pos && pos <= column->viaObject.num);
    
    clVisitChildren(
        handler, 
        column->viaUnion.fields + pos - 1,
        decoder);
}

static inline void VisitFixedArrayHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Decoder *decoder)
{
    const msgpack_object *object = decoder->object;

    CHECK_COND_ERROR(
        decoder,
        object->type == MSGPACK_OBJECT_ARRAY
        && object->via.array.size == column->viaFixedArray.capacity);

    VisitArray(
        handler,
        column->viaFixedArray.capacity,
        column->viaFixedArray.flags,
        column->viaFixedArray.element,
        decoder);
}

static inline void VisitFlexibleArrayHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Decoder *decoder)
{
    const msgpack_object *object = decoder->object;
    const ptrdiff_t offset = decoder->offset;

    CHECK_COND_ERROR(
        decoder,
        object->type == MSGPACK_OBJECT_ARRAY);
    
    const clColumn *prev_column = column - 1;

    const uint32_t num = ReadMemoryAsUInt32(
        prev_column->kind, 
        decoder->addr + offset + prev_column->offset - column->offset);

    CHECK_COND_ERROR(
        decoder,
        object->via.array.size == num);

    VisitArray(
        handler,
        object->via.array.size,
        column->viaFlexibleArray.flags,
        column->viaFlexibleArray.element,
        decoder);
}

static struct clHandler handler = {
    .visitNumber = (clVisitHandler) VisitNumberHandler,
    .visitObject = (clVisitHandler) VisitObjectHandler,
    .visitUnion = (clVisitHandler) VisitUnionHandler,
    .visitFixedArray = (clVisitHandler) VisitFixedArrayHandler,
    .visitFlexibleArray = (clVisitHandler) VisitFlexibleArrayHandler,
};

size_t cFromBuf(
    const clColumn *column, 
    void *addr, 
    size_t size,
    const uint8_t *buf, 
    size_t len)
{
    msgpack_unpacked unpacked;
    msgpack_unpacked_init(&unpacked);

    size_t offset = 0;
    msgpack_unpack_return ret = msgpack_unpack_next(
        &unpacked, 
        (const char *) buf, 
        len, 
        &offset);

    struct Decoder decoder = {
        .hasError = (ret != MSGPACK_UNPACK_SUCCESS),
        .addr = (uint8_t *) addr,
        .size = size,
        .offset = 0,
        .object = &unpacked.data
    };

    if (!decoder.hasError) 
    {
        clVisitChildren(
            &handler,
            column,
            &decoder);
    }

    msgpack_unpacked_destroy(
        &unpacked);

    return !decoder.hasError ?
        offset:
        0;
}
